import request from 'supertest'
import app from '../app.js'
import jwt from 'jsonwebtoken'
import { describe, it, expect, beforeAll } from '@jest/globals'

let tokenFullAccess, tokenNoPermissions
let adminFunctionalityDefinitionId,
	adminObjectDefinitionId,
	adminAccessLevelDefinitionId,
	adminPermissionIdOne,
	adminPermissionIdTwo,
	adminRoleId,
	userId

describe('Admin API', () => {
	beforeAll(async () => {
		const res1 = await request(app)
			.post('/api/user/user-login')
			.send({ username: 'Bob', password: 'bob' })
		tokenFullAccess = res1.body.data.token
		const decoded = jwt.decode(tokenFullAccess)
		if (!decoded || typeof decoded === 'string' || !decoded.user?.id) {
			throw new Error('Invalid token payload')
		}
		userId = decoded.user.id

		const res2 = await request(app)
			.post('/api/user/user-login')
			.send({ username: 'Test', password: 'test' })
		tokenNoPermissions = res2.body.data.token
	})

	it('should create a admin functionality definition', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/adm/adm-functionality-definition')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				name: 'TestDefinition',
				description: 'TestDescription',
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestDefinition')
		adminFunctionalityDefinitionId = res.body.data.id
	})

	it('should create a admin object definition', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/adm/adm-object-definition')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				name: 'TestDefinition',
				description: 'TestDescription',
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestDefinition')
		adminObjectDefinitionId = res.body.data.id
	})

	it('should create a admin access level definition', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/adm/adm-access-level-definition')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				name: 'TestDefinition',
				accessLevel: 0,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestDefinition')
		adminAccessLevelDefinitionId = res.body.data.id
	})

	it('should create a admin role', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/adm/adm-role')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				name: 'TestRole',
				description: 'TestDescription',
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestRole')
		adminRoleId = res.body.data.id
	})

	it('should create a admin role user', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/adm/adm-role-user')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				roleId: adminRoleId,
				userId: userId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.roleId).toBe(adminRoleId)
		expect(res.body.data.userId).toBe(userId)
	})

	it('should get all admin role users', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/adm/adm-role-users')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should delete admin role user', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.delete(`/api/adm/adm-role-user/${adminRoleId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				roleId: adminRoleId,
				userId: userId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.post('/api/adm/adm-role-users')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				roleId: adminRoleId,
				userId: userId,
			})
		expect(getRes.statusCode).toBe(200)
		expect(
			Array.isArray(getRes.body.data) && getRes.body.data.length === 0
		).toBe(true)
	})

	it('should create a admin permission user', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/adm/adm-permission')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				userId: userId,
				admFunctionalityDefinitionId: adminFunctionalityDefinitionId,
				admObjectDefinitionId: adminObjectDefinitionId,
				admAccessLevelDefinitionId: adminAccessLevelDefinitionId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.userId).toBe(userId)
		expect(res.body.data.admFunctionalityDefinitionId).toBe(
			adminFunctionalityDefinitionId
		)
		expect(res.body.data.admObjectDefinitionId).toBe(
			adminObjectDefinitionId
		)
		expect(res.body.data.admAccessLevelDefinitionId).toBe(
			adminAccessLevelDefinitionId
		)
		adminPermissionIdOne = res.body.data.id
	})

	it('should create a admin permission role', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/adm/adm-permission')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				roleId: adminRoleId,
				admFunctionalityDefinitionId: adminFunctionalityDefinitionId,
				admObjectDefinitionId: adminObjectDefinitionId,
				admAccessLevelDefinitionId: adminAccessLevelDefinitionId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.roleId).toBe(adminRoleId)
		expect(res.body.data.admFunctionalityDefinitionId).toBe(
			adminFunctionalityDefinitionId
		)
		expect(res.body.data.admObjectDefinitionId).toBe(
			adminObjectDefinitionId
		)
		expect(res.body.data.admAccessLevelDefinitionId).toBe(
			adminAccessLevelDefinitionId
		)
		adminPermissionIdTwo = res.body.data.id
	})

	it('should return validation error for empty admin permission body', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/adm/adm-permission')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('admFunctionalityDefinitionId')
		expect(res.body.data[1].type).toBe('field')
		expect(res.body.data[1].path).toBe('admAccessLevelDefinitionId')
	})

	it('should not allow user without permissions to create admin permission', async () => {
		const res = await request(app)
			.post('/api/adm/adm-permission')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({
				userId: userId,
				admFunctionalityDefinitionId: adminFunctionalityDefinitionId,
				admObjectDefinitionId: adminObjectDefinitionId,
				admAccessLevelDefinitionId: adminAccessLevelDefinitionId,
			})
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should get admin permissions', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/adm/adm-permissions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(400)
	})

	it('should get admin permissions filtered by userId', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/adm/adm-permissions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ userId: userId })
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get admin permissions filtered by roleId', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/adm/adm-permissions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ roleId: adminRoleId })
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should delete admin permission', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.delete(`/api/adm/adm-permission/${adminPermissionIdOne}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: adminPermissionIdOne,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/adm/adm-permission/${adminPermissionIdOne}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})

	it('should delete admin permission', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.delete(`/api/adm/adm-permission/${adminPermissionIdTwo}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: adminPermissionIdTwo,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/adm/adm-permission/${adminPermissionIdTwo}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})

	it('should return validation error for empty admin functionality definition body', async () => {
		if (tokenFullAccess == null) {
			throw new Error('tokenFullAccess is null')
		}
		const res = await request(app)
			.post('/api/adm/adm-functionality-definition')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('name')
		expect(res.body.data[1].type).toBe('field')
		expect(res.body.data[1].path).toBe('description')
	})

	it('should not allow user without permissions to create admin functionality definition', async () => {
		const res = await request(app)
			.post('/api/adm/adm-functionality-definition')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({
				name: 'TestDefinition',
				description: 'TestDescription',
			})
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the admin functionality definition', async () => {
		const res = await request(app)
			.patch(
				`/api/adm/adm-functionality-definition/${adminFunctionalityDefinitionId}`
			)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				name: 'TestDefinitionEdited',
				description: 'TestDescriptionEdited',
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestDefinitionEdited')
		expect(res.body.data.description).toBe('TestDescriptionEdited')
	})

	it('should get admin functionality definition by id', async () => {
		const res = await request(app)
			.get(
				`/api/adm/adm-functionality-definition/${adminFunctionalityDefinitionId}`
			)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(adminFunctionalityDefinitionId)
	})

	it('should get all admin functionality definitions', async () => {
		const res = await request(app)
			.post('/api/adm/adm-functionality-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get admin functionality definitions filtered by id', async () => {
		const res = await request(app)
			.post('/api/adm/adm-functionality-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: adminFunctionalityDefinitionId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(adminFunctionalityDefinitionId)
	})

	it('should delete admin functionality definition', async () => {
		const res = await request(app)
			.delete(
				`/api/adm/adm-functionality-definition/${adminFunctionalityDefinitionId}`
			)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: adminFunctionalityDefinitionId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(
				`/api/adm/adm-functionality-definition/${adminFunctionalityDefinitionId}`
			)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})

	it('should return validation error for empty admin object definition body', async () => {
		const res = await request(app)
			.post('/api/adm/adm-object-definition')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('name')
		expect(res.body.data[1].type).toBe('field')
		expect(res.body.data[1].path).toBe('description')
	})

	it('should not allow user without permissions to create admin object definition', async () => {
		const res = await request(app)
			.post('/api/adm/adm-object-definition')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({
				name: 'TestDefinition',
				description: 'TestDescription',
			})
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the admin object definition', async () => {
		const res = await request(app)
			.patch(`/api/adm/adm-object-definition/${adminObjectDefinitionId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				name: 'TestDefinitionEdited',
				description: 'TestDescriptionEdited',
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestDefinitionEdited')
		expect(res.body.data.description).toBe('TestDescriptionEdited')
	})

	it('should get admin object definition by id', async () => {
		const res = await request(app)
			.get(`/api/adm/adm-object-definition/${adminObjectDefinitionId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(adminObjectDefinitionId)
	})

	it('should get all admin object definitions', async () => {
		const res = await request(app)
			.post('/api/adm/adm-object-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get admin object definitions filtered by id', async () => {
		const res = await request(app)
			.post('/api/adm/adm-object-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: adminObjectDefinitionId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(adminObjectDefinitionId)
	})

	it('should delete admin object definition', async () => {
		const res = await request(app)
			.delete(`/api/adm/adm-object-definition/${adminObjectDefinitionId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: adminObjectDefinitionId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/adm/adm-object-definition/${adminObjectDefinitionId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})

	it('should return validation error for empty admin access level definition body', async () => {
		const res = await request(app)
			.post('/api/adm/adm-access-level-definition')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('name')
		expect(res.body.data[1].type).toBe('field')
		expect(res.body.data[1].path).toBe('accessLevel')
	})

	it('should not allow user without permissions to create admin access level definition', async () => {
		const res = await request(app)
			.post('/api/adm/adm-access-level-definition')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({
				name: 'TestDefinition',
				accessLevel: 0,
			})
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the admin access level definition', async () => {
		const res = await request(app)
			.patch(
				`/api/adm/adm-access-level-definition/${adminAccessLevelDefinitionId}`
			)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				name: 'TestDefinitionEdited',
				accessLevel: 20,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestDefinitionEdited')
		expect(res.body.data.accessLevel).toBe(20)
	})

	it('should get admin access level definition by id', async () => {
		const res = await request(app)
			.get(
				`/api/adm/adm-access-level-definition/${adminAccessLevelDefinitionId}`
			)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(adminAccessLevelDefinitionId)
	})

	it('should get all admin access level definitions', async () => {
		const res = await request(app)
			.post('/api/adm/adm-access-level-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get admin access level definitions filtered by id', async () => {
		const res = await request(app)
			.post('/api/adm/adm-access-level-definitions')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: adminAccessLevelDefinitionId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(adminAccessLevelDefinitionId)
	})

	it('should delete admin access level definition', async () => {
		const res = await request(app)
			.delete(
				`/api/adm/adm-access-level-definition/${adminAccessLevelDefinitionId}`
			)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: adminAccessLevelDefinitionId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(
				`/api/adm/adm-access-level-definition/${adminAccessLevelDefinitionId}`
			)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})

	it('should return validation error for empty admin role body', async () => {
		const res = await request(app)
			.post('/api/adm/adm-role')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect([400, 422]).toContain(res.statusCode)
		expect(res.body.success).toBe(false)
		expect(res.body.message).toMatch('Validation failed.')
		expect(res.body.data[0].type).toBe('field')
		expect(res.body.data[0].path).toBe('name')
		expect(res.body.data[1].type).toBe('field')
		expect(res.body.data[1].path).toBe('description')
	})

	it('should not allow user without permissions to create admin role', async () => {
		const res = await request(app)
			.post('/api/adm/adm-role')
			.set('Authorization', `Bearer ${tokenNoPermissions}`)
			.send({
				name: 'TestRole',
				description: 'TestDescription',
			})
		expect([401, 403]).toContain(res.statusCode)
	})

	it('should edit the admin role', async () => {
		const res = await request(app)
			.patch(`/api/adm/adm-role/${adminRoleId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				name: 'TestRoleEdited',
				description: 'TestDescriptionEdited',
			})
		expect(res.statusCode).toBe(200)
		expect(res.body.data.name).toBe('TestRoleEdited')
		expect(res.body.data.description).toBe('TestDescriptionEdited')
	})

	it('should get admin role by id', async () => {
		const res = await request(app)
			.get(`/api/adm/adm-role/${adminRoleId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect(res.statusCode).toBe(200)
		expect(res.body.data.id).toBe(adminRoleId)
	})

	it('should get all admin roles', async () => {
		const res = await request(app)
			.post('/api/adm/adm-roles')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({})
		expect(res.statusCode).toBe(200)
		expect(Array.isArray(res.body.data)).toBe(true)
	})

	it('should get admin roles filtered by id', async () => {
		const res = await request(app)
			.post('/api/adm/adm-roles')
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({ id: adminRoleId })
		expect(res.statusCode).toBe(200)
		expect(res.body.data[0].id).toBe(adminRoleId)
	})

	it('should delete admin role', async () => {
		const res = await request(app)
			.delete(`/api/adm/adm-role/${adminRoleId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
			.send({
				id: adminRoleId,
			})
		expect(res.statusCode).toBe(200)
		expect(res.body).toHaveProperty('success', true)

		const getRes = await request(app)
			.get(`/api/adm/adm-role/${adminRoleId}`)
			.set('Authorization', `Bearer ${tokenFullAccess}`)
		expect([404, 503]).toContain(getRes.statusCode)
	})
})
