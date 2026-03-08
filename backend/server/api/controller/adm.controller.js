import sequelize from '../../util/database.js'
import {
	internalServerError,
	serviceUnavailable,
	success,
	unauthorized,
	wrongValidation,
} from '../../util/responseHelper.js'
import { decodeSequelizeQuery } from '../../util/sequelizeTools.js'
import { checkPermission } from './permission.controller.js'
import { getUserDetail } from '../../libs/jwtToken.js'
import { validationResult } from 'express-validator'
import FunctionalityDefinition from '../model/adm/admFunctionalityDefinition.model.js'
import ObjectDefinition from '../model/adm/admObjectDefinition.model.js'
import AccessLevelDefinition from '../model/adm/admAccessLevelDefinitions.model.js'
import AdmRoles from '../model/adm/admRoles.model.js'
import AdmRolesUser from '../model/adm/admRolesUser.model.js'
import AdmPermissions from '../model/adm/admPermissions.js'
import User from '../model/users/user.model.js'

export async function getAdmFunctionalityDefinitions(req, res) {
	try {
		let permissionGranted = await checkPermission(req, 'adm', null, 'READ')
		if (!permissionGranted) return unauthorized(res)
		let queryObject = decodeSequelizeQuery(req.body)
		const data = await FunctionalityDefinition.findAll({
			where: queryObject,
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getAdmFunctionalityDefinition(req, res) {
	try {
		let permissionGranted = await checkPermission(req, 'adm', null, 'READ')
		if (!permissionGranted) return unauthorized(res)
		const admFunctionalityDefinitionId =
			req.params.admFunctionalityDefinitionId
		const data = await FunctionalityDefinition.findByPk(
			admFunctionalityDefinitionId
		)
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addAdmFunctionalityDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admFunctionalityDefinition',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const data = await FunctionalityDefinition.create(req.body, {
			transaction: t,
		})
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

export async function updateAdmFunctionalityDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admFunctionalityDefinition',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const admFunctionalityDefinitionId =
			req.params.admFunctionalityDefinitionId
		const { name, description } = req.body
		const admFunctionalityDefinition =
			await FunctionalityDefinition.findByPk(admFunctionalityDefinitionId)
		if (!admFunctionalityDefinition)
			return serviceUnavailable(res, 'No such item.')
		admFunctionalityDefinition.name = name
		admFunctionalityDefinition.description = description
		const data = await admFunctionalityDefinition.save({ transaction: t })
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

export async function deleteAdmFunctionalityDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admFunctionalityDefinition',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id)
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		let data = await FunctionalityDefinition.destroy({
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

export async function getAdmObjectDefinitions(req, res) {
	try {
		let permissionGranted = await checkPermission(req, 'adm', null, 'READ')
		if (!permissionGranted) return unauthorized(res)
		let queryObject = decodeSequelizeQuery(req.body)
		const data = await ObjectDefinition.findAll({ where: queryObject })
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getAdmObjectDefinition(req, res) {
	try {
		let permissionGranted = await checkPermission(req, 'adm', null, 'READ')
		if (!permissionGranted) return unauthorized(res)
		const admObjectDefinitionId = req.params.admObjectDefinitionId
		const data = await ObjectDefinition.findByPk(admObjectDefinitionId)
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addAdmObjectDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admObjectDefinition',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const data = await ObjectDefinition.create(req.body, { transaction: t })
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

export async function updateAdmObjectDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admObjectDefinition',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const admObjectDefinitionId = req.params.admObjectDefinitionId
		const { name, description } = req.body
		const admObjectDefinition = await ObjectDefinition.findByPk(
			admObjectDefinitionId
		)
		if (!admObjectDefinition)
			return serviceUnavailable(res, 'No such item.')
		admObjectDefinition.name = name
		admObjectDefinition.description = description
		const data = await admObjectDefinition.save({ transaction: t })
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

export async function deleteAdmObjectDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admObjectDefinition',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id)
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		let data = await ObjectDefinition.destroy({
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

export async function getAdmAccessLevelDefinitions(req, res) {
	try {
		let queryObject = decodeSequelizeQuery(req.body)
		const data = await AccessLevelDefinition.findAll({ where: queryObject })
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getAdmAccessLevelDefinition(req, res) {
	try {
		let permissionGranted = await checkPermission(req, 'adm', null, 'READ')
		if (!permissionGranted) return unauthorized(res)
		const admAccessLevelDefinitionId = req.params.admAccessLevelDefinitionId
		const data = await AccessLevelDefinition.findByPk(
			admAccessLevelDefinitionId
		)
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addAdmAccessLevelDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admAccessLevelDefinition',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const data = await AccessLevelDefinition.create(req.body, {
			transaction: t,
		})
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

export async function updateAdmAccessLevelDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admAccessLevelDefinition',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const admAccessLevelDefinitionId = req.params.admAccessLevelDefinitionId
		const { name, accessLevel } = req.body
		const admAccessLevelDefinition = await AccessLevelDefinition.findByPk(
			admAccessLevelDefinitionId
		)
		if (!admAccessLevelDefinition)
			return serviceUnavailable(res, 'No such item.')
		admAccessLevelDefinition.name = name
		admAccessLevelDefinition.accessLevel = accessLevel
		const data = await admAccessLevelDefinition.save({ transaction: t })
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

export async function deleteAdmAccessLevelDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admAccessLevelDefinition',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id)
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		let data = await AccessLevelDefinition.destroy({
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

export async function getAdmRoles(req, res) {
	try {
		let permissionGranted = await checkPermission(req, 'adm', null, 'READ')
		if (!permissionGranted) return unauthorized(res)
		let queryObject = decodeSequelizeQuery(req.body)
		const data = await AdmRoles.findAll({
			where: queryObject,
			include: ['createdBy', 'updatedBy', 'users'],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getAdmRole(req, res) {
	try {
		let permissionGranted = await checkPermission(req, 'adm', null, 'READ')
		if (!permissionGranted) return unauthorized(res)
		const admRoleId = req.params.admRoleId
		const data = await AdmRoles.findByPk(admRoleId, {
			include: ['createdBy', 'updatedBy', 'users'],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addAdmRole(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admRole',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const user = getUserDetail(req)
		let queryObject = {
			...req.body,
			createdById: user.id,
			updatedById: user.id,
		}
		const data = await AdmRoles.create(queryObject, { transaction: t })
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

export async function updateAdmRole(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admRole',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const admRoleId = req.params.admRoleId
		const { name, description } = req.body
		const user = getUserDetail(req)
		const updatedById = user.id
		const admRole = await AdmRoles.findByPk(admRoleId)
		if (!admRole) return serviceUnavailable(res, 'No such item.')
		admRole.name = name
		admRole.description = description
		admRole.updatedById = updatedById
		const data = await admRole.save({ transaction: t })
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

export async function deleteAdmRole(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admRole',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id)
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		let data = await AdmRoles.destroy({
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

export async function getAdmRoleUsers(req, res) {
	try {
		let queryObject = decodeSequelizeQuery(req.body)
		const data = await AdmRolesUser.findAll({
			where: queryObject,
			include: ['role'],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addAdmRoleUser(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admRoleUser',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const admRole = await AdmRoles.findByPk(req.body.roleId)
		if (!admRole) return serviceUnavailable(res, 'No such item.')
		const user = await User.findByPk(req.body.userId)
		if (!user) return serviceUnavailable(res, 'No such item.')
		const data = await AdmRolesUser.create(req.body, { transaction: t })
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

export async function deleteAdmRoleUser(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admRoleUser',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.roleId || !req.body.userId)
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		let data = await AdmRolesUser.destroy({
			where: { roleId: req.body.roleId, userId: req.body.userId },
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

export async function addAdmPermission(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admPermission',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		if (!req.body.userId && !req.body.roleId)
			return serviceUnavailable(res, 'No such item.')
		if (req.body.userId) {
			const user = await User.findByPk(req.body.userId)
			if (!user) return serviceUnavailable(res, 'No such item.')
		}
		if (req.body.roleId) {
			const admRole = await AdmRoles.findByPk(req.body.roleId)
			if (!admRole) return serviceUnavailable(res, 'No such item.')
		}
		const admFunctionalityDefinition =
			await FunctionalityDefinition.findByPk(
				req.body.admFunctionalityDefinitionId
			)
		if (!admFunctionalityDefinition)
			return serviceUnavailable(res, 'No such item.')
		if (req.body.admObjectDefinitionId) {
			const admObjectDefinition = await ObjectDefinition.findByPk(
				req.body.admObjectDefinitionId
			)
			if (!admObjectDefinition)
				return serviceUnavailable(res, 'No such item.')
		}
		const admAccessLevel = await AccessLevelDefinition.findByPk(
			req.body.admAccessLevelDefinitionId
		)
		if (!admAccessLevel) return serviceUnavailable(res, 'No such item.')
		const data = await AdmPermissions.create(req.body, { transaction: t })
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

export async function deleteAdmPermission(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admPermission',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id)
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		let data = await AdmPermissions.destroy({
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
