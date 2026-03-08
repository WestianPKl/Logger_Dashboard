import request from 'supertest'
import app from '../app.js'
import { describe, it, expect, beforeAll } from '@jest/globals'

let tokenFullAccess, tokenNoPermissions
let modelId,
	vendorId,
	typeId,
	equipmentId,
	sensorId,
	sensorTypeId,
	sensorVendorId,
	sensorModelId
let temperatureDefinitionId, humidityDefinitionId

describe('Equipment API', () => {
	beforeAll(async () => {
		const res1 = await request(app)
			.post('/api/user/user-login')
			.send({ username: 'Bob', password: 'bob' })
		tokenFullAccess = res1.body.data.token

		const res2 = await request(app)
			.post('/api/user/user-login')
			.send({ username: 'Test', password: 'test' })
		tokenNoPermissions = res2.body.data.token
	})

	it('should create a model', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/equipment/equ-model')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'TestModel' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestModel')
		modelId = res.body.data.id
	})

	it('should return validation error for empty model name', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/equipment/equ-model')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: '' })
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('name')
	})

	it('should not allow user without permissions to create model', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-model')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({ name: 'TestModel' })
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the model', async () => {
		const res = await request(app)
			.patch(`/api/equipment/equ-model/${modelId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'TestModelEdited' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestModelEdited')
	})

	it('should get model by id', async () => {
		const res = await request(app)
			.get(`/api/equipment/equ-model/${modelId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(modelId)
	})

	it('should get all models', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-models')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get models filtered by id', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-models')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: modelId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(modelId)
	})

	it('should create a vendor', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-vendor')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'TestVendor' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestVendor')
		vendorId = res.body.data.id
	})

	it('should return validation error for empty vendor name', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-vendor')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: '' })
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('name')
	})

	it('should not allow user without permissions to create vendor', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-vendor')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({ name: 'TestVendor' })
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the vendor', async () => {
		const res = await request(app)
			.patch(`/api/equipment/equ-vendor/${vendorId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'TestVendorEdited' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestVendorEdited')
	})

	it('should get vendor by id', async () => {
		const res = await request(app)
			.get(`/api/equipment/equ-vendor/${vendorId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(vendorId)
	})

	it('should get all vendors', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-vendors')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get vendors filtered by id', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-vendors')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: vendorId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(vendorId)
	})

	it('should create a type', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-type')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'TestType' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestType')
		typeId = res.body.data.id
	})

	it('should return validation error for empty type name', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-type')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: '' })
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('name')
	})

	it('should not allow user without permissions to create type', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-type')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({ name: 'TestType' })
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the type', async () => {
		const res = await request(app)
			.patch(`/api/equipment/equ-type/${typeId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'TestTypeEdited' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestTypeEdited')
	})

	it('should get type by id', async () => {
		const res = await request(app)
			.get(`/api/equipment/equ-type/${typeId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(typeId)
	})

	it('should get all types', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-types')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get types filtered by id', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-types')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: typeId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(typeId)
	})

	it('should create an equipment', async () => {
		const res = await request(app)
			.post('/api/equipment/equipment')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				serialNumber: 'SN-001',
				equVendorId: vendorId,
				equModelId: modelId,
				equTypeId: typeId,
			})
		expect(res.statusCode).toBe(200)
		equipmentId = res.body.data.id
	})

	it('should return validation error for empty equipment body', async () => {
		const res = await request(app)
			.post('/api/equipment/equipment')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('serialNumber')
		expect(res.body.data[1].type).toBe('field')
		expect(res.body.data[1].path).toBe('equVendorId')
		expect(res.body.data[3].type).toBe('field')
		expect(res.body.data[3].path).toBe('equModelId')
		expect(res.body.data[5].type).toBe('field')
		expect(res.body.data[5].path).toBe('equTypeId')
	})

	it('should not allow user without permissions to create equipment', async () => {
		const res = await request(app)
			.post('/api/equipment/equipment')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({
				serialNumber: 'SN-001',
				equVendorId: vendorId,
				equModelId: modelId,
				equTypeId: typeId,
			})
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the equipment', async () => {
		const res = await request(app)
			.patch(`/api/equipment/equipment/${equipmentId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				serialNumber: 'SN-001-EDIT',
				equVendorId: vendorId,
				equModelId: modelId,
				equTypeId: typeId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.serialNumber).toBe('SN-001-EDIT')
	})

	it('should get equipment by id', async () => {
		const res = await request(app)
			.get(`/api/equipment/equipment/${equipmentId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(equipmentId)
	})

	it('should get all equipments', async () => {
		const res = await request(app)
			.post('/api/equipment/equipments')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get equipments filtered by id', async () => {
		const res = await request(app)
			.post('/api/equipment/equipments')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: equipmentId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(equipmentId)
	})

	it('should not allow user without permissions to create equipment', async () => {
		const res = await request(app)
			.post('/api/equipment/equipment')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({
				serialNumber: 'SN-BLOCKED',
				equVendorId: vendorId,
				equModelId: modelId,
				equTypeId: typeId,
			})
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should return validation error for empty serialNumber', async () => {
		const res = await request(app)
			.post('/api/equipment/equipment')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				serialNumber: '',
				equVendorId: vendorId,
				equModelId: modelId,
				equTypeId: typeId,
			})
		expect([400, 422]).toContain(res.statusCode)
	})

	it('should delete equipment', async () => {
		const res = await request(app)
			.delete(`/api/equipment/equipment/${equipmentId}`)
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

	it('should restore equipment', async () => {
		const res = await request(app)
			.patch(`/api/equipment/equipment-restore/${equipmentId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: equipmentId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		await request(app)
			.get(`/api/equipment/equipment/${equipmentId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
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

	it('should delete model', async () => {
		const res = await request(app)
			.delete(`/api/equipment/equ-model/${modelId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: modelId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/equipment/equ-model/${modelId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})

	it('should delete vendor', async () => {
		const res = await request(app)
			.delete(`/api/equipment/equ-vendor/${vendorId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: vendorId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/equipment/equ-vendor/${vendorId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})

	it('should delete type', async () => {
		const res = await request(app)
			.delete(`/api/equipment/equ-type/${typeId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: typeId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/equipment/equ-type/${typeId}`)
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

	it('should get sensor model', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-models')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'SHT-41' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].name).toBe('SHT-41')
		sensorModelId = res.body.data[0].id
	})

	it('should get sensor vendor', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-vendors')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'Sensirion' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].name).toBe('Sensirion')
		sensorVendorId = res.body.data[0].id
	})

	it('should create an equipment', async () => {
		const res = await request(app)
			.post('/api/equipment/equipment')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				serialNumber: 'SN-001',
				equVendorId: sensorVendorId,
				equModelId: sensorModelId,
				equTypeId: sensorTypeId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.equTypeId).toBe(sensorTypeId)
		sensorId = res.body.data.id
	})

	it('should get data definition TEMPERATURE', async () => {
		const res = await request(app)
			.post('/api/data/data-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'temperature' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].name).toBe('temperature')
		temperatureDefinitionId = res.body.data[0].id
	})

	it('should get data definition TEMPERATURE', async () => {
		const res = await request(app)
			.post('/api/data/data-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'humidity' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].name).toBe('humidity')
		humidityDefinitionId = res.body.data[0].id
	})

	it('should assign sensor function "temperature"', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-sensor-function')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				equSensorId: sensorId,
				dataDefinitionId: temperatureDefinitionId,
			})
		expect(res.statusCode).toBe(200)
	})

	it('should assign sensor function "humidity"', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-sensor-function')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				equSensorId: sensorId,
				dataDefinitionId: humidityDefinitionId,
			})
		expect(res.statusCode).toBe(200)
	})

	it('should delete sensor function temperature', async () => {
		const res = await request(app)
			.delete(`/api/equipment/equ-sensor-function/${sensorId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				equSensorId: sensorId,
				dataDefinitionId: temperatureDefinitionId,
			})
		expect(res.statusCode).toBe(200)
	})

	it('should delete sensor function humidity', async () => {
		const res = await request(app)
			.delete(`/api/equipment/equ-sensor-function/${sensorId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				equSensorId: sensorId,
				dataDefinitionId: humidityDefinitionId,
			})
		expect(res.statusCode).toBe(200)
	})

	it('should delete equipment forced', async () => {
		const res = await request(app)
			.delete(`/api/equipment/equipment-forced/${sensorId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: sensorId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/equipment/equipment/${equipmentId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})

	it('should get all unused equipments', async () => {
		const res = await request(app)
			.post('/api/equipment/equ-unused-loggers')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})
})
