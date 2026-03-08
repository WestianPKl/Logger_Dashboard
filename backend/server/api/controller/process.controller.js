import sequelize from '../../util/database.js'
import {
	internalServerError,
	badRequest,
	serviceUnavailable,
	success,
	unauthorized,
	wrongValidation,
} from '../../util/responseHelper.js'
import { decodeSequelizeQuery } from '../../util/sequelizeTools.js'
import { checkPermission } from './permission.controller.js'
import { getUserDetail } from '../../libs/jwtToken.js'
import { validationResult } from 'express-validator'
import ProcessType from '../model/process/processType.model.js'
import ProcessDefinition from '../model/process/processDefinition.model.js'

export async function getProcessTypes(req, res) {
	try {
		const permissionGranded = await checkPermission(
			req,
			'process',
			null,
			'READ'
		)
		if (!permissionGranded) {
			return unauthorized(res)
		}
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await ProcessType.findAll({ where: queryObject })
		if (!data) {
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getProcessType(req, res) {
	try {
		const permissionGranded = await checkPermission(
			req,
			'process',
			null,
			'READ'
		)
		if (!permissionGranded) {
			return unauthorized(res)
		}
		const processTypeId = req.params.processTypeId
		const data = await ProcessType.findByPk(processTypeId)
		if (!data) {
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addProcessType(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranded = await checkPermission(
			req,
			'process',
			'processType',
			'WRITE'
		)
		if (!permissionGranded) {
			return unauthorized(res)
		}
		const errors = validationResult(req)
		if (!errors.isEmpty()) {
			return wrongValidation(res, 'Validation failed.', errors.array())
		}
		const data = await ProcessType.create(req.body, { transaction: t })
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

export async function updateProcessType(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranded = await checkPermission(
			req,
			'process',
			'processType',
			'WRITE'
		)
		if (!permissionGranded) {
			return unauthorized(res)
		}
		const errors = validationResult(req)
		if (!errors.isEmpty()) {
			return wrongValidation(res, 'Validation failed.', errors.array())
		}
		const processTypeId = req.params.processTypeId
		const { name } = req.body
		const processType = await ProcessType.findByPk(processTypeId)
		if (!processType) {
			return serviceUnavailable(res, 'No such item.')
		}
		processType.name = name
		const data = await processType.save({ transaction: t })
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

export async function deleteProcessType(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranded = await checkPermission(
			req,
			'process',
			'processType',
			'DELETE'
		)
		if (!permissionGranded) {
			return unauthorized(res)
		}
		const { id } = req.body
		if (!id) {
			return badRequest(res, 'Deleting data failed - no ID.')
		}
		const data = await ProcessType.destroy({
			where: { id },
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

export async function getProcessDefinitions(req, res) {
	try {
		const permissionGranded = await checkPermission(
			req,
			'process',
			null,
			'READ'
		)
		if (!permissionGranded) {
			return unauthorized(res)
		}
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await ProcessDefinition.findAll({
			where: queryObject,
			include: ['type', 'createdBy', 'updatedBy'],
		})
		if (!data) {
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getProcessDefinition(req, res) {
	try {
		const permissionGranded = await checkPermission(
			req,
			'process',
			null,
			'READ'
		)
		if (!permissionGranded) {
			return unauthorized(res)
		}
		const processDefinitionId = req.params.processDefinitionId
		const data = await ProcessDefinition.findByPk(processDefinitionId, {
			include: ['type', 'createdBy', 'updatedBy'],
		})
		if (!data) {
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addProcessDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranded = await checkPermission(
			req,
			'process',
			'processDefinition',
			'WRITE'
		)
		if (!permissionGranded) {
			return unauthorized(res)
		}
		const errors = validationResult(req)
		if (!errors.isEmpty()) {
			return wrongValidation(res, 'Validation failed.', errors.array())
		}
		const user = getUserDetail(req)
		const queryObject = {
			...req.body,
			createdById: user.id,
			updatedById: user.id,
		}
		const data = await ProcessDefinition.create(queryObject, {
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

export async function updateProcessDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranded = await checkPermission(
			req,
			'process',
			'processDefinition',
			'WRITE'
		)
		if (!permissionGranded) {
			return unauthorized(res)
		}
		const errors = validationResult(req)
		if (!errors.isEmpty()) {
			return wrongValidation(res, 'Validation failed.', errors.array())
		}
		const processDefinitionId = req.params.processDefinitionId
		const { processTypeId, name } = req.body
		const user = getUserDetail(req)
		const updatedById = user.id
		const processDefinition =
			await ProcessDefinition.findByPk(processDefinitionId)
		if (!processDefinition) {
			return serviceUnavailable(res, 'No such item.')
		}
		processDefinition.processTypeId = processTypeId
		processDefinition.name = name
		processDefinition.updatedById = updatedById
		const data = await processDefinition.save({ transaction: t })
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

export async function deleteProcessDefinition(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranded = await checkPermission(
			req,
			'process',
			'processDefinition',
			'DELETE'
		)
		if (!permissionGranded) {
			return unauthorized(res)
		}
		const { id } = req.body
		if (!id) {
			return badRequest(res, 'Deleting data failed - no ID.')
		}
		const data = await ProcessDefinition.destroy({
			where: { id },
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
