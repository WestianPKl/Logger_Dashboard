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
import EquType from '../model/equipment/equType.model.js'
import EquVendor from '../model/equipment/equVendor.model.js'
import EquModel from '../model/equipment/equModel.model.js'
import Equipment from '../model/equipment/equipment.model.js'
import EquUnusedLoggerView from '../model/equipment/equUnusedLogger.model.js'
import EquSensorFunctions from '../model/equipment/equSensorsFunctions.model.js'
import EquStats from '../model/equipment/equStats.model.js'
import EquLogs from '../model/equipment/equLogs.model.js'

export async function getEquTypes(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await EquType.findAll({ where: queryObject })
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getEquType(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const equTypeId = req.params.equTypeId
		const data = await EquType.findByPk(equTypeId)
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addEquType(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equType',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const data = await EquType.create(req.body, { transaction: t })
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

export async function updateEquType(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equType',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const equTypeId = req.params.equTypeId
		const name = req.body.name
		const equType = await EquType.findByPk(equTypeId)
		if (!equType) {
			await t.rollback()
			return serviceUnavailable(res, 'No such item.')
		}
		equType.name = name
		const data = await equType.save({ transaction: t })
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

export async function deleteEquType(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equType',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id) {
			await t.rollback()
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		}
		let data = await EquType.destroy({
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

export async function getEquVendors(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await EquVendor.findAll({ where: queryObject })
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getEquVendor(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const equVendorId = req.params.equVendorId
		const data = await EquVendor.findByPk(equVendorId)
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addEquVendor(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equVendor',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const data = await EquVendor.create(req.body, { transaction: t })
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

export async function updateEquVendor(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equVendor',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const equVendorId = req.params.equVendorId
		const name = req.body.name
		const equVendor = await EquVendor.findByPk(equVendorId)
		if (!equVendor) {
			await t.rollback()
			return serviceUnavailable(res, 'No such item.')
		}
		equVendor.name = name
		const data = await equVendor.save({ transaction: t })
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

export async function deleteEquVendor(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equVendor',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id) {
			await t.rollback()
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		}
		let data = await EquVendor.destroy({
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

export async function getEquModels(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await EquModel.findAll({ where: queryObject })
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getEquModel(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const equModelId = req.params.equModelId
		const data = await EquModel.findByPk(equModelId)
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addEquModel(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equModel',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const data = await EquModel.create(req.body, { transaction: t })
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

export async function updateEquModel(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equModel',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const equModelId = req.params.equModelId
		const name = req.body.name
		const equModel = await EquModel.findByPk(equModelId)
		if (!equModel) {
			await t.rollback()
			return serviceUnavailable(res, 'No such item.')
		}
		equModel.name = name
		const data = await equModel.save({ transaction: t })
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

export async function deleteEquModel(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equModel',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id) {
			await t.rollback()
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		}
		let data = await EquModel.destroy({
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

export async function getEquipments(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		let queryObject
		if (Array.isArray(req.body) && req.body.length > 0) {
			queryObject = { id: req.body }
		} else if (req.body.equTypeId) {
			queryObject = { equTypeId: req.body.equTypeId }
		} else {
			queryObject = decodeSequelizeQuery(req.body)
		}
		const data = await Equipment.findAll({
			where: queryObject,
			include: [
				'vendor',
				'model',
				'type',
				'createdBy',
				'updatedBy',
				'dataDefinitions',
			],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getEquipmentsAdmin(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		let queryObject
		if (Array.isArray(req.body) && req.body.length > 0) {
			queryObject = { id: req.body }
		} else if (req.body.equTypeId) {
			queryObject = { equTypeId: req.body.equTypeId }
		} else {
			queryObject = decodeSequelizeQuery(req.body)
		}
		const data = await Equipment.findAll({
			where: queryObject,
			paranoid: false,
			include: [
				'vendor',
				'model',
				'type',
				'createdBy',
				'updatedBy',
				'dataDefinitions',
			],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getEquipment(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const equipmentId = req.params.equipmentId
		const data = await Equipment.findByPk(equipmentId, {
			include: [
				'vendor',
				'model',
				'type',
				'createdBy',
				'updatedBy',
				'dataDefinitions',
				'logs',
				'stats',
				'errors',
			],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addEquipment(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equEquipment',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const user = getUserDetail(req)
		const dataDefinitions = req.body.dataDefinitions
		let queryObject = {
			...req.body,
			createdById: user.id,
			updatedById: user.id,
		}
		const data = await Equipment.create(queryObject, { transaction: t })
		const sensor = await EquType.findByPk(req.body.equTypeId, { raw: true })
		if (
			sensor?.name === 'Sensor' &&
			Array.isArray(dataDefinitions) &&
			dataDefinitions.length > 0
		) {
			for (const item of dataDefinitions) {
				await EquSensorFunctions.create(
					{ equSensorId: data.id, dataDefinitionId: item.id },
					{ transaction: t }
				)
			}
		}
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

export async function updateEquipment(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equEquipment',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const equipmentId = req.params.equipmentId
		const {
			serialNumber,
			equModelId,
			equVendorId,
			equTypeId,
			dataDefinitions,
		} = req.body
		const user = getUserDetail(req)
		const equipment = await Equipment.findByPk(equipmentId, {
			include: ['type'],
		})
		if (!equipment) {
			await t.rollback()
			return serviceUnavailable(res, 'No such item.')
		}
		equipment.serialNumber = serialNumber
		equipment.equModelId = equModelId
		equipment.equVendorId = equVendorId
		equipment.equTypeId = equTypeId
		equipment.updatedById = user.id
		await EquSensorFunctions.destroy({
			where: { equSensorId: equipmentId },
			transaction: t,
		})
		if (
			equipment.type?.name === 'Sensor' &&
			Array.isArray(dataDefinitions) &&
			dataDefinitions.length > 0
		) {
			for (const item of dataDefinitions) {
				await EquSensorFunctions.create(
					{ equSensorId: equipment.id, dataDefinitionId: item.id },
					{ transaction: t }
				)
			}
		}
		const data = await equipment.save({ transaction: t })
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

export async function deleteEquipment(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equEquipment',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id) {
			await t.rollback()
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		}
		const equ = await Equipment.findByPk(req.body.id)
		if (!equ) {
			await t.rollback()
			return serviceUnavailable(
				res,
				'Deleting data failed - no equipment.'
			)
		}
		const data = await Equipment.destroy({
			where: { id: req.body.id },
			transaction: t,
		})
		const user = getUserDetail(req)
		equ.updatedById = user.id
		equ.isDeleted = 1
		await equ.save({ transaction: t })
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

export async function restoreEquipment(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equEquipment',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id) {
			await t.rollback()
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		}
		const equ = await Equipment.findByPk(req.body.id, { paranoid: false })
		if (!equ) {
			await t.rollback()
			return serviceUnavailable(
				res,
				'Deleting data failed - no equipment.'
			)
		}
		const data = await Equipment.restore(
			{ where: { id: req.body.id } },
			{ transaction: t }
		)
		if (!data) {
			await t.rollback()
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		const user = getUserDetail(req)
		equ.updatedById = user.id
		equ.isDeleted = 0
		await equ.save({ transaction: t })
		await t.commit()
		return success(res, 'Data deleted successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function deleteEquipmentForced(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equEquipment',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id) {
			await t.rollback()
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		}
		let data = await Equipment.destroy({
			where: { id: req.body.id },
			force: true,
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

export async function getEquUnusedLoggers(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await EquUnusedLoggerView.findAll({ where: queryObject })
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addEquSensorFunction(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equEquipment',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const equSensor = await Equipment.findByPk(req.body.equSensorId, {
			include: ['vendor', 'model', 'type'],
		})
		if (!equSensor || equSensor.type?.name !== 'Sensor') {
			await t.rollback()
			return serviceUnavailable(res, 'No such item.')
		}
		const data = await EquSensorFunctions.create(req.body, {
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

export async function deleteEquSensorFunction(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equEquipment',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.equSensorId || !req.body.dataDefinitionId) {
			await t.rollback()
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		}
		let data = await EquSensorFunctions.destroy({
			where: {
				equSensorId: req.body.equSensorId,
				dataDefinitionId: req.body.dataDefinitionId,
			},
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

export async function getEquStats(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await EquStats.findAll({ where: queryObject })
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getEquStat(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const equStatId = req.params.equStatId
		const data = await EquStats.findByPk(equStatId)
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addEquStat(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equ',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const data = await EquStats.create(req.body, { transaction: t })
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

export async function updateEquStat(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equ',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const equStatId = req.params.equStatId
		const equStat = await EquStats.findByPk(equStatId)
		if (!equStat) {
			await t.rollback()
			return serviceUnavailable(res, 'No such item.')
		}
		equStat.lastSeen = req.body.lastSeen ?? equStat.lastSeen
		equStat.snContr = req.body.snContr ?? equStat.snContr
		equStat.fwContr = req.body.fwContr ?? equStat.fwContr
		equStat.hwContr = req.body.hwContr ?? equStat.hwContr
		equStat.buildContr = req.body.buildContr ?? equStat.buildContr
		equStat.prodContr = req.body.prodContr ?? equStat.prodContr
		equStat.snCom = req.body.snCom ?? equStat.snCom
		equStat.fwCom = req.body.fwCom ?? equStat.fwCom
		equStat.hwCom = req.body.hwCom ?? equStat.hwCom
		equStat.buildCom = req.body.buildCom ?? equStat.buildCom
		equStat.prodCom = req.body.prodCom ?? equStat.prodCom
		equStat.ipAddress = req.body.ipAddress ?? equStat.ipAddress
		const data = await equStat.save({ transaction: t })
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

export async function deleteEquStat(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equ',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id) {
			await t.rollback()
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		}
		let data = await EquStats.destroy({
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

export async function getEquLogs(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await EquLogs.findAll({ where: queryObject })
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getEquLog(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const equLogId = req.params.equLogId
		const data = await EquLogs.findByPk(equLogId)
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addEquLog(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equ',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const data = await EquLogs.create(req.body, { transaction: t })
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

export async function updateEquLog(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equ',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const equLogId = req.params.equLogId
		const equLog = await EquLogs.findByPk(equLogId)
		if (!equLog) {
			await t.rollback()
			return serviceUnavailable(res, 'No such item.')
		}
		equLog.equipmentId = req.body.equipmentId ?? equLog.equipmentId
		equLog.message = req.body.message ?? equLog.message
		equLog.type = req.body.type ?? equLog.type
		const data = await equLog.save({ transaction: t })
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

export async function deleteEquLog(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'equ',
			'equ',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id) {
			await t.rollback()
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		}
		let data = await EquLogs.destroy({
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
