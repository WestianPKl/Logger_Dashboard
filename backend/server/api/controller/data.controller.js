import sequelize from '../../util/database.js'
import {
	internalServerError,
	serviceUnavailable,
	success,
	unauthorized,
	wrongValidation,
} from '../../util/responseHelper.js'
import { getIo } from '../../middleware/socket.js'
import { decodeSequelizeQuery } from '../../util/sequelizeTools.js'
import { checkPermission } from './permission.controller.js'
import { getUserDetail, generateAccessToken } from '../../libs/jwtToken.js'
import { validationResult } from 'express-validator'
import DataDefinitions from '../model/data/dataDefinitions.model.js'
import DataLastValue from '../model/data/dataLastValue.model.js'
import DataLogs from '../model/data/dataLogs.model.js'
import DataLastValueView from '../model/data/dataLastValueView.model.js'
import DataConnectedSensorView from '../model/data/dataConnectedSensorView.model.js'
import DataLogsView from '../model/data/dataLogsView.model.js'

export async function getDataDefinitions(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await DataDefinitions.findAll({ where: queryObject })
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getDataDefinition(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const dataDefinitionId = req.params.dataDefinitionId
		const data = await DataDefinitions.findByPk(dataDefinitionId)
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addDataDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			'dataDefinition',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const data = await DataDefinitions.create(req.body, { transaction: t })
		if (!data) {
			await t.rollback()
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		await t.commit()
		return success(res, 'Data added successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function updateDataDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			'dataDefinition',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const dataDefinitionId = req.params.dataDefinitionId
		const { name, unit, description } = req.body
		const dataDefinition = await DataDefinitions.findByPk(dataDefinitionId)
		if (!dataDefinition) return serviceUnavailable(res, 'No such item.')
		dataDefinition.name = name
		dataDefinition.unit = unit
		dataDefinition.description = description
		const data = await dataDefinition.save({ transaction: t })
		if (!data) {
			await t.rollback()
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		await t.commit()
		return success(res, 'Data updated successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function deleteDataDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			'dataDefinition',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id)
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		const data = await DataDefinitions.destroy({
			where: { id: req.body.id },
			transaction: t,
		})
		if (!data) {
			await t.rollback()
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		await t.commit()
		return success(res, 'Data deleted successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getDataLogs(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await DataLogs.findAll({
			where: queryObject,
			include: ['logger', 'sensor', 'definition'],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getDataLog(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const dataLogId = req.params.dataLogId
		const data = await DataLogs.findByPk(dataLogId, {
			include: ['logger', 'sensor', 'definition'],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addDataLog(req, res) {
	const t = await sequelize.transaction()
	try {
		const user = getUserDetail(req)
		if (user.username !== 'logger') {
			const permissionGranted = await checkPermission(
				req,
				'data',
				'dataLog',
				'WRITE'
			)
			if (!permissionGranted) return unauthorized(res)
		}
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		let addData = null
		let data = []
		let dataLastValue = null
		const dataLog = req.body
		if (Array.isArray(dataLog) && dataLog.length > 0) {
			await DataLastValue.destroy({
				where: {
					equLoggerId: dataLog[0].equLoggerId,
					equSensorId: dataLog[0].equSensorId,
				},
				transaction: t,
			})
			for (const item of dataLog) {
				if (!item.definition) {
					return serviceUnavailable(res, 'No data definition name.')
				}
				const definitionData = await DataDefinitions.findOne({
					where: { name: item.definition },
				})
				const queryObject = {
					...item,
					dataDefinitionId: definitionData.id,
				}
				addData = await DataLogs.create(queryObject, { transaction: t })
				dataLastValue = await DataLastValue.create(
					{
						dataLogId: addData.id,
						equLoggerId: addData.equLoggerId,
						equSensorId: addData.equSensorId,
						dataDefinitionId: addData.dataDefinitionId,
					},
					{ transaction: t }
				)
				data.push(addData)
			}
		} else {
			if (!req.body.definition) {
				return serviceUnavailable(res, 'No data definition name.')
			}
			const definitionData = await DataDefinitions.findOne({
				where: { name: req.body.definition },
			})
			const queryObject = {
				...req.body,
				dataDefinitionId: definitionData.id,
			}
			addData = await DataLogs.create(queryObject, { transaction: t })
			data.push(addData)
			await DataLastValue.destroy({
				where: {
					equLoggerId: addData.equLoggerId,
					equSensorId: addData.equSensorId,
					dataDefinitionId: addData.dataDefinitionId,
				},
				transaction: t,
			})
			dataLastValue = await DataLastValue.create(
				{
					dataLogId: addData.id,
					equLoggerId: addData.equLoggerId,
					equSensorId: addData.equSensorId,
					dataDefinitionId: addData.dataDefinitionId,
				},
				{ transaction: t }
			)
		}
		if (!data && !dataLastValue) {
			await t.rollback()
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		await t.commit()
		return success(res, 'Data added successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function updateDataLog(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			'dataLog',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const dataLogId = req.params.dataLogId
		const { value, dataDefinitionId, equLoggerId, equSensorId, time } =
			req.body
		const dataLog = await DataLogs.findByPk(dataLogId)
		if (!dataLog) return serviceUnavailable(res, 'No such item.')
		dataLog.value = value
		dataLog.dataDefinitionId = dataDefinitionId
		dataLog.equLoggerId = equLoggerId
		dataLog.equSensorId = equSensorId
		dataLog.time = time
		const data = await dataLog.save({ transaction: t })
		if (!data) {
			await t.rollback()
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		await t.commit()
		return success(res, 'Data updated successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function deleteDataLog(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			'dataLog',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id)
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		const dataLastValues = await DataLastValue.findAll({
			where: { dataLogId: req.body.id },
		})
		let lastValue = 1
		if (dataLastValues && dataLastValues.length > 0) {
			lastValue = await DataLastValue.destroy({
				where: { dataLogId: req.body.id },
				transaction: t,
			})
		}
		const data = await DataLogs.destroy({
			where: { id: req.body.id },
			transaction: t,
		})
		if (!data && !lastValue) {
			await t.rollback()
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		await t.commit()
		return success(res, 'Data deleted successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getDataLastValues(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await DataLastValue.findAll({
			where: queryObject,
			include: [
				'logger',
				{
					model: DataLogs,
					as: 'log',
					include: ['sensor', 'definition'],
				},
			],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getDataLastValue(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const dataLastValueId = req.params.dataLastValueId
		const data = await DataLastValue.findByPk(dataLastValueId, {
			include: [
				'logger',
				{
					model: DataLogs,
					as: 'log',
					include: ['sensor', 'definition'],
				},
			],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addDataLastValue(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			'dataLog',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const data = await DataLastValue.create(req.body, { transaction: t })
		if (!data) {
			await t.rollback()
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		await t.commit()
		let io
		try {
			io = getIo()
		} catch {
			io = null
		}
		if (io) io.sockets.emit(`loggerData_${data.equLoggerId}`, 'refresh')
		return success(res, 'Data added successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function updateDataLastValue(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			'dataLog',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const dataLastValueId = req.params.dataLastValueId
		const dataLogId = req.body.dataLogId
		const dataLastValue = await DataLastValue.findByPk(dataLastValueId)
		if (!dataLastValue) return serviceUnavailable(res, 'No such item.')
		dataLastValue.dataLogId = dataLogId
		const data = await dataLastValue.save({ transaction: t })
		if (!data) {
			await t.rollback()
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		await t.commit()
		return success(res, 'Data updated successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function deleteDataLastValue(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			'dataLog',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id)
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		const data = await DataLastValue.destroy({
			where: { id: req.body.id },
			transaction: t,
		})
		if (!data) {
			await t.rollback()
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		await t.commit()
		return success(res, 'Data deleted successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getDataLastValuesView(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await DataLastValueView.findAll({ where: queryObject })
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getDataConnectedSensorView(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await DataConnectedSensorView.findAll({
			where: queryObject,
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getDataLogsView(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await DataLogsView.findAll({ where: queryObject })
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getLastValuesForLoggers(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'data',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const { loggerIds } = req.body
		if (!Array.isArray(loggerIds) || loggerIds.length === 0) {
			return serviceUnavailable(
				res,
				'Must be an array of equipment loggers IDs.'
			)
		}
		const data = await DataLastValueView.findAll({
			where: { equLoggerId: loggerIds },
			order: [['time', 'DESC']],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getDataToken(req, res) {
	const token = generateAccessToken({
		tokenType: 3,
		user: { id: 0, username: 'logger', email: '', password: '' },
	})
	res.status(200).json({ token })
}
