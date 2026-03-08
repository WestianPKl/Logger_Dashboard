import mqtt from 'mqtt'
import Equipment from './api/model/equipment/equipment.model.js'
import sequelize from './util/database.js'
import DataLastValue from './api/model/data/dataLastValue.model.js'
import DataDefinitions from './api/model/data/dataDefinitions.model.js'
import DataLogs from './api/model/data/dataLogs.model.js'
import EquStats from './api/model/equipment/equStats.model.js'
import EquLogs from './api/model/equipment/equLogs.model.js'
import { getIo } from './middleware/socket.js'

const MQTT_URL =
	process.env.NODE_ENV === 'production'
		? process.env.MQTT_URL_PROD
		: process.env.MQTT_URL_DEV
const MQTT_USER =
	process.env.NODE_ENV === 'production'
		? process.env.MQTT_USER_PROD
		: process.env.MQTT_USER_DEV
const MQTT_PASS =
	process.env.NODE_ENV === 'production'
		? process.env.MQTT_PASS_PROD
		: process.env.MQTT_PASS_DEV

export const mqttClient = mqtt.connect(MQTT_URL, {
	username: MQTT_USER,
	password: MQTT_PASS,
	reconnectPeriod: 2000,
	keepalive: 30,
	clean: true,
})

async function updateStatus(obj, t) {
	if (obj.info.logger_id && obj.info.sensor_id) {
		const logger = await Equipment.findByPk(obj.info.logger_id, {
			transaction: t,
		})
		const sensor = await Equipment.findByPk(obj.info.sensor_id, {
			transaction: t,
		})

		if (!logger || !sensor) {
			console.warn('[MQTT] Logger or sensor not found for:', obj.sn_contr)
			await t.rollback()
			return
		}

		await EquLogs.create(
			{
				equipmentId: logger.id,
				message: `Status update: ${JSON.stringify(obj.info)}`,
				type: obj.type,
			},
			{ transaction: t }
		)

		await EquStats.update(
			{ lastSeen: obj.info.rtc },
			{
				where: { equipmentId: logger.id },
				transaction: t,
			}
		)
	}
}

mqttClient.on('reconnect', () => console.log('[MQTT] reconnecting...'))
mqttClient.on('error', (e) => console.log('[MQTT] error:', e.message))

mqttClient.on('connect', () => {
	console.log('[MQTT] connected:', MQTT_URL)
	mqttClient.subscribe('devices/+/status', { qos: 0 }, (err) => {
		if (err) console.log('[MQTT] subscribe error:', err.message)
		else console.log('[MQTT] subscribed to devices/+/status')
	})
})

mqttClient.on('message', async (topic, payload) => {
	const s = payload.toString('utf8')
	const t = await sequelize.transaction()
	try {
		const obj = JSON.parse(s)
		if (obj.info && obj.info.rtc) {
			if (new Date(obj.info.rtc).getFullYear() < 2024) {
				obj.info.rtc = new Date().toLocaleString('sv-SE', {
					timeZone: 'Europe/Warsaw',
				})
			} else {
				obj.info.rtc = new Date(obj.info.rtc).toLocaleString('sv-SE', {
					timeZone: 'Europe/Warsaw',
				})
			}
		}

		if (obj.type == 'STATUS') {
			updateStatus(obj, t)
				.then(() => t.commit())
				.catch((err) => {
					console.error('[MQTT] Error updating status:', err)
					t.rollback()
				})
		}

		if (obj.type == 'ERROR') {
			console.log(obj)
		}

		if (obj.type == 'DATA') {
			const dataLog = []
			if (obj.info.logger_id && obj.info.sensor_id) {
				const logger = await Equipment.findByPk(obj.info.logger_id, {
					transaction: t,
				})
				const sensor = await Equipment.findByPk(obj.info.sensor_id, {
					transaction: t,
				})

				if (!logger || !sensor) {
					console.warn(
						'[MQTT] Logger or sensor not found for:',
						obj.sn_contr
					)
					await t.rollback()
					return
				}

				if (obj.info.sht40) {
					Object.entries(obj.info.sht40).forEach(([type, value]) => {
						dataLog.push({
							time: obj.info.rtc,
							value: value,
							definition: type,
							equLoggerId: logger.id,
							equSensorId: sensor.id,
						})
					})
				}

				if (obj.info.bme280) {
					Object.entries(obj.info.bme280).forEach(([type, value]) => {
						if (type === 'pressure') {
							type = 'atmPressure'
						}
						dataLog.push({
							time: obj.info.rtc,
							value: value,
							definition: type,
							equLoggerId: logger.id,
							equSensorId: sensor.id,
						})
					})
				}

				if (
					obj.info.vin &&
					Array.isArray(obj.info.vin) &&
					obj.info.vin.length > 0
				) {
					obj.info.vin.forEach((vin) => {
						dataLog.push({
							time: obj.info.rtc,
							value: vin,
							definition: `voltage`,
							equLoggerId: logger.id,
							equSensorId: sensor.id,
						})
					})
				}

				const equStatsLogs = {
					equipmentId: logger.id,
					lastSeen: obj.info.rtc,
					snContr: obj.info.controller_serial ?? 0,
					fwContr: obj.info.controller_sw ?? '',
					hwContr: obj.info.controller_hw ?? '',
					buildContr: obj.info.controller_build_date ?? '',
					prodContr: obj.info.controller_prod_date ?? '',
					fwCom: obj.info.communication_sw ?? '',
					buildCom: obj.info.communication_build ?? '',
					ipAddress: obj.info.ip_address ?? '',
				}

				if (dataLog && Array.isArray(dataLog) && dataLog.length > 0) {
					await DataLastValue.destroy({
						where: {
							equLoggerId: dataLog[0].equLoggerId,
							equSensorId: dataLog[0].equSensorId,
						},
						transaction: t,
					})
					for (const item of dataLog) {
						if (!item.definition) {
							await t.rollback()
							return
						}
						const definitionData = await DataDefinitions.findOne({
							where: { name: item.definition },
						})
						if (!definitionData) {
							console.warn(
								'[MQTT] Definition not found:',
								item.definition
							)
							await t.rollback()
							return
						}
						const queryObject = {
							...item,
							dataDefinitionId: definitionData.id,
						}
						const addData = await DataLogs.create(queryObject, {
							transaction: t,
						})
						await DataLastValue.create(
							{
								dataLogId: addData.id,
								equLoggerId: addData.equLoggerId,
								equSensorId: addData.equSensorId,
								dataDefinitionId: addData.dataDefinitionId,
							},
							{ transaction: t }
						)
					}
				}

				if (equStatsLogs.equipmentId) {
					await EquStats.destroy({
						where: {
							equipmentId: equStatsLogs.equipmentId,
						},
						transaction: t,
					})

					await EquStats.create(equStatsLogs, { transaction: t })
				}

				await t.commit()
				let io
				try {
					io = getIo()
				} catch {
					io = null
				}
				if (io) io.sockets.emit(`logger_${logger.id}`, 'refresh')
				if (io) io.sockets.emit(`loggerData_${logger.id}`, 'refresh')
			}
		}
	} catch (err) {
		await t.rollback()
		console.error('[MQTT] Error processing message:', err)
	}
})
