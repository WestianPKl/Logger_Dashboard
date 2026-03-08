import request from 'supertest'
import app from '../app.js'
import { describe, it, expect, beforeAll } from '@jest/globals'

let tokenFullAccess, tokenNoPermissions
let processTypeId, processDefinitionId

describe('Process API', () => {
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

	it('should create a process type', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/process/process-type')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'TestType' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestType')
		processTypeId = res.body.data.id
	})

	it('should return validation error for empty process type name', async () => {
		const res = await request(app)
			.post('/api/process/process-type')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: '' })
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('name')
	})

	it('should not allow user without permissions to create process type', async () => {
		const res = await request(app)
			.post('/api/process/process-type')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({ name: 'TestType' })
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the process type', async () => {
		const res = await request(app)
			.patch(`/api/process/process-type/${processTypeId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'TestTypeEdited' })
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestTypeEdited')
	})

	it('should get process type by id', async () => {
		const res = await request(app)
			.get(`/api/process/process-type/${processTypeId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(processTypeId)
	})

	it('should get all process types', async () => {
		const res = await request(app)
			.post('/api/process/process-types')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get process types filtered by id', async () => {
		const res = await request(app)
			.post('/api/process/process-types')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: processTypeId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(processTypeId)
	})

	it('should create a process definition', async () => {
		const res = await request(app)
			.post('/api/process/process-definition')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: 'TestDefinition', processTypeId: processTypeId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestDefinition')
		processDefinitionId = res.body.data.id
	})

	it('should return validation error for empty process definition name', async () => {
		const res = await request(app)
			.post('/api/process/process-definition')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ name: '', processTypeId: processTypeId })
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('name')
	})

	it('should not allow user without permissions to create process definition', async () => {
		const res = await request(app)
			.post('/api/process/process-definition')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({ name: 'TestDefiniton', processTypeId: processTypeId })
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the process definition', async () => {
		const res = await request(app)
			.patch(`/api/process/process-definition/${processDefinitionId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				name: 'TestDefinitionEdited',
				processTypeId: processTypeId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestDefinitionEdited')
	})

	it('should get process definition by id', async () => {
		const res = await request(app)
			.get(`/api/process/process-definition/${processDefinitionId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(processDefinitionId)
	})

	it('should get all process definitions', async () => {
		const res = await request(app)
			.post('/api/process/process-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get process definitions filtered by id', async () => {
		const res = await request(app)
			.post('/api/process/process-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: processDefinitionId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(processDefinitionId)
	})

	it('should delete process definition', async () => {
		const res = await request(app)
			.delete(`/api/process/process-definition/${processDefinitionId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: processDefinitionId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/process/process-definition/${processDefinitionId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})

	it('should delete process type', async () => {
		const res = await request(app)
			.delete(`/api/process/process-type/${processTypeId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: processTypeId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/process/process-type/${processTypeId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})
})
