import request from 'supertest'
import { jest } from '@jest/globals'
import { describe, it, expect, beforeAll } from '@jest/globals'

let app

let tokenFullAccess, tokenNoPermissions
let houseId,
	houseFloorId,
	houseLoggerId,
	equipmentId,
	loggerTypeId,
	loggerModelId,
	loggerVendorId,
	secondHouseFloorId

describe('House API', () => {
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

	it('should create a house', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/house/house')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				name: 'TestHouse',
				postalCode: '00-000',
				city: 'TestCity',
				street: 'TestStreet',
				houseNumber: '1',
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestHouse')
		houseId = res.body.data.id
	})

	it('should return validation error for empty house body', async () => {
		const res = await request(app)
			.post('/api/house/house')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('name')
		expect(res.body.data[1].type).toBe('field')
		expect(res.body.data[1].path).toBe('houseNumber')
	})

	it('should not allow user without permissions to create house', async () => {
		const res = await request(app)
			.post('/api/house/house')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({
				name: 'TestHouse',
				postalCode: '00-000',
				city: 'TestCity',
				street: 'TestStreet',
				houseNumber: '1',
			})
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the house', async () => {
		const res = await request(app)
			.patch(`/api/house/house/${houseId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				name: 'TestHouseEdited',
				postalCode: '00-000Edited',
				city: 'TestCityEdited',
				street: 'TestStreetEdited',
				houseNumber: '1Edited',
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestHouseEdited')
		expect(res.body.data.postalCode).toBe('00-000Edited')
		expect(res.body.data.city).toBe('TestCityEdited')
		expect(res.body.data.street).toBe('TestStreetEdited')
		expect(res.body.data.houseNumber).toBe('1Edited')
	})

	it('should get house by id', async () => {
		const res = await request(app)
			.get(`/api/house/house/${houseId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(houseId)
	})

	it('should get all houses', async () => {
		const res = await request(app)
			.post('/api/house/houses')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get houses filtered by id', async () => {
		const res = await request(app)
			.post('/api/house/houses')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: houseId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(houseId)
	})

	it('should create a house floor', async () => {
		const res = await request(app)
			.post('/api/house/house-floor')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'TestFloor', houseId: houseId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestFloor')
		houseFloorId = res.body.data.id
	})

	it('should return validation error for empty house floor name', async () => {
		const res = await request(app)
			.post('/api/house/house-floor')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: '' })
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('name')
	})

	it('should not allow user without permissions to create house floor', async () => {
		const res = await request(app)
			.post('/api/house/house-floor')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({ name: 'TestFloor', houseId: houseId })
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the house floor', async () => {
		const res = await request(app)
			.patch(`/api/house/house-floor/${houseFloorId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'TestFloorEdited', houseId: houseId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestFloorEdited')
	})

	it('should get house floor by id', async () => {
		const res = await request(app)
			.get(`/api/house/house-floor/${houseFloorId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(houseFloorId)
	})

	it('should get all house floors', async () => {
		const res = await request(app)
			.post('/api/house/house-floors')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get house floors filtered by id', async () => {
		const res = await request(app)
			.post('/api/house/house-floors')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: houseFloorId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(houseFloorId)
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

	it('should get logger model', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-models')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'Pico' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].name).toBe('Pico')
		loggerModelId = res.body.data[0].id
	})

	it('should get logger vendor', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-vendors')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'Raspberry' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].name).toBe('Raspberry')
		loggerVendorId = res.body.data[0].id
	})

	it('should create an equipment', async () => {
		const res = await request(app)
			.post('/api/equipment/equipment')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				serialNumber: 'SN-001',
				equVendorId: loggerVendorId,
				equModelId: loggerModelId,
				equTypeId: loggerTypeId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.equTypeId).toBe(loggerTypeId)
		equipmentId = res.body.data.id
	})

	it('should create a house logger', async () => {
		const res = await request(app)
			.post('/api/house/house-logger')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ equLoggerId: equipmentId, houseFloorId: houseFloorId })
		expect(res.statusCode).toBe(200)
		houseLoggerId = res.body.data.id
	})

	it('should return validation error for empty house logger houseFloor ID', async () => {
		const res = await request(app)
			.post('/api/house/house-logger')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ houseFloorId: houseFloorId })
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('equLoggerId')
	})

	it('should not allow user without permissions to create house logger', async () => {
		const res = await request(app)
			.post('/api/house/house-logger')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({ equLoggerId: equipmentId, houseFloorId: houseFloorId })
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should get all house floors', async () => {
		const res = await request(app)
			.post('/api/house/house-floors')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
		secondHouseFloorId = res.body.data[0].id
	})

	it('should edit the house logger', async () => {
		const res = await request(app)
			.patch(`/api/house/house-logger/${houseLoggerId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				equLoggerId: equipmentId,
				houseFloorId: secondHouseFloorId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.houseFloorId).toBe(secondHouseFloorId)
	})

	it('should get house logger by id', async () => {
		const res = await request(app)
			.get(`/api/house/house-logger/${houseLoggerId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(houseLoggerId)
	})

	it('should get all house loggers', async () => {
		const res = await request(app)
			.post('/api/house/house-loggers')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get house loggers filtered by id', async () => {
		const res = await request(app)
			.post('/api/house/house-loggers')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: houseLoggerId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(houseLoggerId)
	})

	it('should delete house logger', async () => {
		const res = await request(app)
			.delete(`/api/house/house-logger/${houseLoggerId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: houseLoggerId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/house/house-logger/${houseLoggerId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})

	it('should delete house floor', async () => {
		const res = await request(app)
			.delete(`/api/house/house-floor/${houseFloorId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: houseFloorId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/house/house-floor/${houseFloorId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})

	it('should delete house', async () => {
		const res = await request(app)
			.delete(`/api/house/house/${houseId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: houseId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/house/house/${houseId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})

	it('should delete equipment forced', async () => {
		const res = await request(app)
			.delete(`/api/equipment/equipment-forced/${equipmentId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: equipmentId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/equipment/equipment/${equipmentId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})
})
