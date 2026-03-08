import request from 'supertest'
import { jest } from '@jest/globals'
import { describe, it, expect, beforeAll } from '@jest/globals'

let app

let tokenFullAccess, tokenNoPermissions
let dataDefinitionId,
	loggerId,
	sensorId,
	sensorTypeId,
	loggerTypeId,
	loggerToken,
	dataLogIdOne,
	dataLogIdTwo,
	dataLastValueId,
	temperatureDataDefinitionId

describe('Data API', () => {
	beforeAll(async () => {
		jest.unstable_mockModule('../middleware/socket.js', () => ({
			getIo: () => ({ sockets: { emit: jest.fn() } }),
		}))
		const appModule = await import('../app.js')
		app = appModule.default

		const res1 = await request(app)
			.post('/api/user/user-login')
			.send({ username: 'Bob', password: 'bob' })
		tokenFullAccess = res1.body.data.token

		const res2 = await request(app)
			.post('/api/user/user-login')
			.send({ username: 'Test', password: 'test' })
		tokenNoPermissions = res2.body.data.token
	})

	it('should create a data definition', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/data/data-definition')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				name: 'TestDefinition',
				unit: 'TestUnit',
				description: 'TestDescription',
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestDefinition')
		dataDefinitionId = res.body.data.id
	})

	it('should return validation error for empty data definition body', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/data/data-definition')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('name')
		expect(res.body.data[1].type).toBe('field')
		expect(res.body.data[1].path).toBe('unit')
		expect(res.body.data[2].type).toBe('field')
		expect(res.body.data[2].path).toBe('description')
	})

	it('should not allow user without permissions to create data definition', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/data/data-definition')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({
				name: 'TestDefinition',
				unit: 'TestUnit',
				description: 'TestDescription',
			})
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the data definition', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.patch(`/api/data/data-definition/${dataDefinitionId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				name: 'TestDefinitionEdited',
				unit: 'TestUnitEdited',
				description: 'TestDescriptionEdited',
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestDefinitionEdited')
		expect(res.body.data.unit).toBe('TestUnitEdited')
		expect(res.body.data.description).toBe('TestDescriptionEdited')
	})

	it('should get data definition by id', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.get(`/api/data/data-definition/${dataDefinitionId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(dataDefinitionId)
	})

	it('should get all data definitions', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/data/data-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get data definitions filtered by id', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/data/data-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: dataDefinitionId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(dataDefinitionId)
	})

	it('should delete data definition', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.delete(`/api/data/data-definition/${dataDefinitionId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: dataDefinitionId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/data/data-definition/${dataDefinitionId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})

	it('should get sensor type', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-types')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'Sensor' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].name).toBe('Sensor')
		sensorTypeId = res.body.data[0].id
	})

	it('should get logger type', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-types')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'Logger' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].name).toBe('Logger')
		loggerTypeId = res.body.data[0].id
	})

	it('should get equipments filtered by logger type', async () => {
		const res = await request(app)
			.post('/api/equipment/equipments')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ equTypeId: loggerTypeId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].equTypeId).toBe(loggerTypeId)
		loggerId = res.body.data[0].id
	})

	it('should get equipments filtered by sensor type', async () => {
		const res = await request(app)
			.post('/api/equipment/equipments')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ equTypeId: sensorTypeId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].equTypeId).toBe(sensorTypeId)
		sensorId = res.body.data[0].id
	})

	it('should get data definitions filtered by temperature name', async () => {
		const res = await request(app)
			.post('/api/data/data-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'temperature' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].name).toBe('temperature')
		temperatureDataDefinitionId = res.body.data[0].id
	})

	it('should get data definitions filtered by humidity name', async () => {
		const res = await request(app)
			.post('/api/data/data-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'humidity' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].name).toBe('humidity')
		// humidityDataDefinitionId = res.body.data[0].id
	})

	it('should get logger token', async () => {
		const res = await request(app).get('/api/data/data-token')
		expect(res.statusCode).toBe(200)
		loggerToken = res.body.token
	})

	it('should create a data log', async () => {
		const res = await request(app)
			.post('/api/data/data-log')
			.set('Authorization', `Bearer ${loggerToken}`)
			.send([
				{
					value: 20,
					definition: 'temperature',
					equLoggerId: loggerId,
					equSensorId: sensorId,
					time: '2025-06-01 12:00:00',
				},
				{
					value: 50,
					definition: 'humidity',
					equLoggerId: loggerId,
					equSensorId: sensorId,
					time: '2025-06-01 12:00:00',
				},
			])
		expect(res.statusCode).toBe(200)
		dataLogIdOne = res.body.data[0].id
		dataLogIdTwo = res.body.data[1].id
	})

	it('should return validation error for empty data definition log', async () => {
		const res = await request(app)
			.post('/api/data/data-log')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect([400, 422, 503]).toContain(res.statusCode)
	})

	it('should not allow user without permissions to create data log', async () => {
		const res = await request(app)
			.post('/api/data/data-log')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send([
				{
					value: 20,
					definition: 'temperature',
					equLoggerId: loggerId,
					equSensorId: sensorId,
					time: '2025-06-01 12:00:00',
				},
				{
					value: 50,
					definition: 'humidity',
					equLoggerId: loggerId,
					equSensorId: sensorId,
					time: '2025-06-01 12:00:00',
				},
			])
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the data log', async () => {
		const res = await request(app)
			.patch(`/api/data/data-log/${dataLogIdOne}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				value: 30,
				dataDefinitionId: temperatureDataDefinitionId,
				equLoggerId: loggerId,
				equSensorId: sensorId,
				time: '2025-06-01 13:00:00',
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.value).toBe(30)
	})

	it('should get data log by id', async () => {
		const res = await request(app)
			.get(`/api/data/data-log/${dataLogIdOne}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(dataLogIdOne)
	})

	it('should get data logs filtered by id', async () => {
		const res = await request(app)
			.post('/api/data/data-logs')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: dataLogIdOne })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(dataLogIdOne)
	})

	it('should create a data last value', async () => {
		const res = await request(app)
			.post('/api/data/data-last-value')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				dataLogId: dataLogIdOne,
				equLoggerId: loggerId,
				equSensorId: sensorId,
				dataDefinitionId: temperatureDataDefinitionId,
			})
		expect(res.statusCode).toBe(200)
		dataLastValueId = res.body.data.id
	})

	it('should get data last value by id', async () => {
		const res = await request(app)
			.get(`/api/data/data-last-value/${dataLastValueId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(dataLastValueId)
	})

	it('should get all last values', async () => {
		const res = await request(app)
			.post('/api/data/data-last-values')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get last values filtered by id', async () => {
		const res = await request(app)
			.post('/api/data/data-last-values')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: dataLastValueId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(dataLastValueId)
	})

	it('should get all data connected sensors', async () => {
		const res = await request(app)
			.post('/api/data/data-connected-sensor-view')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ equLoggerId: loggerId })
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get all data last values view', async () => {
		const res = await request(app)
			.post('/api/data/data-last-values-view')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ equLoggerId: loggerId })
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get all data logs view', async () => {
		const res = await request(app)
			.post('/api/data/data-logs-view')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ equLoggerId: loggerId })
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get all data logs multi', async () => {
		const res = await request(app)
			.post('/api/data/data-last-values-multi')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ loggerIds: [loggerId] })
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should not get all data logs multi (not array)', async () => {
		const res = await request(app)
			.post('/api/data/data-last-values-multi')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ loggerIds: loggerId })
		expect(res.statusCode).toBe(503)
	})

	it('should delete data log', async () => {
		const res = await request(app)
			.delete(`/api/data/data-log/${dataLogIdOne}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: dataLogIdOne,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/data/data-log/${dataLogIdOne}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 500, 503]).toContain(getRes.statusCode)
	})

	it('should delete data log', async () => {
		const res = await request(app)
			.delete(`/api/data/data-log/${dataLogIdTwo}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: dataLogIdTwo,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/data/data-log/${dataLogIdTwo}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 500, 503]).toContain(getRes.statusCode)
	})
})
