import sequelize from '../../util/database.js'
import {
	internalServerError,
	serviceUnavailable,
	success,
	wrongValidation,
} from '../../util/responseHelper.js'
import { decodeSequelizeQuery } from '../../util/sequelizeTools.js'
import { validationResult } from 'express-validator'
import ErrorLog from '../model/errorLog.model.js'

export async function getErrorLogs(req, res) {
	try {
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await ErrorLog.findAll({ where: queryObject })
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getErrorLog(req, res) {
	try {
		const errorLogId = req.params.errorLogId
		const data = await ErrorLog.findByPk(errorLogId)
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addErrorLog(req, res) {
	const t = await sequelize.transaction()
	try {
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const data = await ErrorLog.create(req.body, { transaction: t })
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

export async function updateErrorLog(req, res) {
	const t = await sequelize.transaction()
	try {
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const errorLogId = req.params.errorLogId
		const message = req.body.message
		const details = req.body.details
		const type = req.body.type
		const severity = req.body.severity
		const equipmentId = req.body.equipmentId
		const errorLog = await ErrorLog.findByPk(errorLogId)
		if (!errorLog) {
			await t.rollback()
			return serviceUnavailable(res, 'No such item.')
		}
		errorLog.message = message
		errorLog.details = details
		errorLog.type = type
		errorLog.severity = severity
		errorLog.equipmentId = equipmentId
		const data = await errorLog.save({ transaction: t })
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

export async function deleteErrorLog(req, res) {
	const t = await sequelize.transaction()
	try {
		if (!req.body.id) {
			await t.rollback()
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		}
		let data = await ErrorLog.destroy({
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
