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
import { getUserDetail } from '../../libs/jwtToken.js'
import { validationResult } from 'express-validator'
import HouseLogger from '../model/house/houseLogger.model.js'
import Equipment from '../model/equipment/equipment.model.js'
import House from '../model/house/house.model.js'
import HouseFloors from '../model/house/houseFloors.model.js'
import { deleteFile } from '../../middleware/file.js'
import path from 'path'
import sharp from 'sharp'

export async function getHouses(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await House.findAll({
			where: queryObject,
			include: ['createdBy', 'updatedBy'],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getHouse(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const houseId = req.params.houseId
		const data = await House.findByPk(houseId, {
			include: [
				{
					model: HouseFloors,
					as: 'floors',
					include: [
						{
							model: HouseLogger,
							as: 'loggers',
							include: [
								{
									model: Equipment,
									as: 'logger',
									include: ['vendor', 'model'],
								},
							],
						},
					],
				},
				'createdBy',
				'updatedBy',
			],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addHouse(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			'houseHouse',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const user = getUserDetail(req)
		let pictureLink = null
		let pictureLinkBig = null
		if (req.file !== undefined) {
			pictureLinkBig = req.file.path
			const ext = path.extname(req.file.filename)
			const thumbPath = req.file.path.replace(ext, `.thumb${ext}`)
			await sharp(req.file.path)
				.resize({
					width: 300,
					height: 300,
					fit: 'inside',
					withoutEnlargement: true,
				})
				.toFile(thumbPath)
			pictureLink = thumbPath
		}
		let queryObject = {
			...req.body,
			pictureLink: pictureLink,
			pictureLinkBig: pictureLinkBig,
			createdById: user.id,
			updatedById: user.id,
		}
		const data = await House.create(queryObject, { transaction: t })
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
		if (io) {
			io.sockets.emit('house', 'add')
			io.sockets.emit(`house-${data.id}`, 'add')
		}
		return success(res, 'Data added successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function updateHouse(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			'houseHouse',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const houseId = req.params.houseId
		const { name, postalCode, city, street, houseNumber } = req.body
		const user = getUserDetail(req)
		const house = await House.findByPk(houseId)
		if (!house) return serviceUnavailable(res, 'No such item.')
		house.name = name
		house.postalCode = postalCode
		house.city = city
		house.street = street
		house.houseNumber = houseNumber
		house.updatedById = user.id

		if (req.file !== undefined) {
			if (house.pictureLink) {
				try {
					deleteFile(house.pictureLink)
				} catch (err) {
					console.log('Picture (thumb) delete error:', err)
				}
			}
			if (house.pictureLink) {
				try {
					deleteFile(house.pictureLink)
				} catch (err) {
					console.log('PictureBig delete error:', err)
				}
			}
			house.pictureLinkBig = req.file.path
			const ext = path.extname(req.file.filename)
			const thumbPath = req.file.path.replace(ext, `.thumb${ext}`)
			await sharp(req.file.path)
				.resize({
					width: 300,
					height: 300,
					fit: 'inside',
					withoutEnlargement: true,
				})
				.toFile(thumbPath)

			house.pictureLink = thumbPath
		}

		const data = await house.save({ transaction: t })
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
		if (io) {
			io.sockets.emit('house', 'update')
			io.sockets.emit(`house-${houseId}`, 'update')
		}
		return success(res, 'Data updated successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function deleteHouse(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			'houseHouse',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id)
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		const house = await House.findOne({ where: { id: req.body.id } })
		if (house && house.pictureLink) {
			try {
				deleteFile(house.pictureLink)
			} catch (err) {
				console.log(err)
			}
		}

		if (house && house.pictureLinkBig) {
			try {
				deleteFile(house.pictureLinkBig)
			} catch (err) {
				console.log(err)
			}
		}

		const data = await House.destroy({
			where: { id: req.body.id },
			transaction: t,
		})
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
		if (io) {
			io.sockets.emit('house', 'delete')
			io.sockets.emit(`house-${req.body.id}`, 'delete')
		}
		return success(res, 'Data deleted successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getHouseFloors(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await HouseFloors.findAll({
			where: queryObject,
			include: ['house'],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getHouseFloor(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const houseFloorId = req.params.houseFloorId
		const data = await HouseFloors.findByPk(houseFloorId, {
			include: ['house'],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addHouseFloor(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			'houseFloor',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const user = getUserDetail(req)
		let layout = null
		let layoutBig = null
		if (req.file !== undefined) {
			layoutBig = req.file.path
			const ext = path.extname(req.file.filename)
			const thumbPath = req.file.path.replace(ext, `.thumb${ext}`)
			await sharp(req.file.path)
				.resize({
					width: 800,
					height: 800,
					fit: 'inside',
					withoutEnlargement: true,
				})
				.toFile(thumbPath)
			layout = thumbPath
		}
		let queryObject = {
			...req.body,
			layout: layout,
			layoutBig: layoutBig,
			createdById: user.id,
			updatedById: user.id,
		}
		const data = await HouseFloors.create(queryObject, { transaction: t })
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
		if (io) io.sockets.emit(`house-${data.houseId}`, 'floorAdd')
		return success(res, 'Data added successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function updateHouseFloor(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			'houseFloor',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const houseFloorId = req.params.houseFloorId
		const { name, houseId } = req.body
		const user = getUserDetail(req)
		const houseFloor = await HouseFloors.findByPk(houseFloorId)
		if (!houseFloor) return serviceUnavailable(res, 'No such item.')
		houseFloor.name = name
		houseFloor.houseId = houseId
		houseFloor.updatedById = user.id

		if (req.file !== undefined) {
			if (houseFloor.layout) {
				try {
					deleteFile(houseFloor.layout)
				} catch (err) {
					console.log('Layout (thumb) delete error:', err)
				}
			}
			if (houseFloor.layoutBig) {
				try {
					deleteFile(houseFloor.layoutBig)
				} catch (err) {
					console.log('LayoutBig delete error:', err)
				}
			}
			houseFloor.layoutBig = req.file.path
			const ext = path.extname(req.file.filename)
			const thumbPath = req.file.path.replace(ext, `.thumb${ext}`)
			await sharp(req.file.path)
				.resize({
					width: 800,
					height: 800,
					fit: 'inside',
					withoutEnlargement: true,
				})
				.toFile(thumbPath)

			houseFloor.layout = thumbPath
		}
		const data = await houseFloor.save({ transaction: t })
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
		if (io) io.sockets.emit(`house-${houseId}`, 'floorUpdate')
		return success(res, 'Data updated successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

async function updateLoggerPosition(loggers) {
	if (Array.isArray(loggers) && loggers.length > 0) {
		for (const e of loggers) {
			let logger = await HouseLogger.findByPk(e.id)
			if (logger) {
				logger.posX = e.posX
				logger.posY = e.posY
				await logger.save()
			}
		}
	}
}

export async function updateHouseFloorLayout(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			'houseFloor',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const houseFloorId = req.params.houseFloorId
		const { x, y, loggers, zoom, posX, posY } = req.body
		const user = getUserDetail(req)
		const houseFloor = await HouseFloors.findByPk(houseFloorId)
		if (!houseFloor) return serviceUnavailable(res, 'No such item.')
		houseFloor.updatedById = user.id
		houseFloor.x = x
		houseFloor.y = y
		houseFloor.zoom = zoom
		houseFloor.posX = posX
		houseFloor.posY = posY
		const data = await houseFloor.save({ transaction: t })
		await updateLoggerPosition(loggers)
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

export async function deleteHouseFloor(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			'houseFloor',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id)
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		const houseFloor = await HouseFloors.findOne({
			where: { id: req.body.id },
		})
		if (houseFloor && houseFloor.layout) {
			try {
				deleteFile(houseFloor.layout)
			} catch (err) {
				console.log(err)
			}
		}
		if (houseFloor && houseFloor.layoutBig) {
			try {
				deleteFile(houseFloor.layoutBig)
			} catch (err) {
				console.log(err)
			}
		}
		const data = await HouseFloors.destroy({
			where: { id: req.body.id },
			transaction: t,
		})
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
		if (io && houseFloor)
			io.sockets.emit(`house-${houseFloor.houseId}`, 'floorDelete')
		return success(res, 'Data deleted successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getHouseLoggers(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const queryObject = decodeSequelizeQuery(req.body)
		const data = await HouseLogger.findAll({
			where: queryObject,
			include: [
				{
					model: Equipment,
					as: 'logger',
					include: ['vendor', 'model', 'type'],
				},
				'floor',
			],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getHouseLogger(req, res) {
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			null,
			'READ'
		)
		if (!permissionGranted) return unauthorized(res)
		const houseLoggerId = req.params.houseLoggerId
		const data = await HouseLogger.findByPk(houseLoggerId, {
			include: [
				{
					model: Equipment,
					as: 'logger',
					include: ['vendor', 'model', 'type'],
				},
				'floor',
			],
		})
		if (!data) return serviceUnavailable(res, 'Retrieving data failed.')
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function addHouseLogger(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			'houseLogger',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const equLogger = await Equipment.findByPk(req.body.equLoggerId, {
			include: ['vendor', 'model', 'type'],
		})
		if (!equLogger || !equLogger.type || equLogger.type.name !== 'Logger') {
			return serviceUnavailable(res, 'No such item.')
		}
		const data = await HouseLogger.create(req.body, { transaction: t })
		if (!data) {
			await t.rollback()
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		await t.commit()
		const logger = await HouseLogger.findByPk(data.id, {
			include: [
				{
					model: Equipment,
					as: 'logger',
					include: ['vendor', 'model', 'type'],
				},
				'floor',
			],
		})
		let io
		try {
			io = getIo()
		} catch {
			io = null
		}
		if (io && logger?.floor?.houseId)
			io.sockets.emit(`house-${logger.floor.houseId}`, 'loggerAdd')
		return success(res, 'Data added successfully', logger)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function updateHouseLogger(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			'houseLogger',
			'WRITE'
		)
		if (!permissionGranted) return unauthorized(res)
		const errors = validationResult(req)
		if (!errors.isEmpty())
			return wrongValidation(res, 'Validation failed.', errors.array())
		const houseLoggerId = req.params.houseLoggerId
		const { equLoggerId, houseFloorId, posX, posY } = req.body
		const houseLogger = await HouseLogger.findByPk(houseLoggerId)
		if (!houseLogger) return serviceUnavailable(res, 'No such item.')
		const houseFloor = await HouseFloors.findByPk(houseFloorId)
		if (!houseFloor) return serviceUnavailable(res, 'No such item.')
		const equLogger = await Equipment.findByPk(equLoggerId, {
			include: ['vendor', 'model', 'type'],
		})
		if (!equLogger || !equLogger.type || equLogger.type.name !== 'Logger') {
			return serviceUnavailable(res, 'No such item.')
		}
		houseLogger.equLoggerId = equLoggerId
		houseLogger.houseFloorId = houseFloorId
		houseLogger.posX = posX
		houseLogger.posY = posY
		const data = await houseLogger.save({ transaction: t })
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
		if (io) io.sockets.emit(`house-${houseFloor.houseId}`, 'loggerUpdate')
		return success(res, 'Data updated successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function deleteHouseLogger(req, res) {
	const t = await sequelize.transaction()
	try {
		const permissionGranted = await checkPermission(
			req,
			'house',
			'houseLogger',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		if (!req.body.id)
			return serviceUnavailable(res, 'Deleting data failed - no ID.')
		const houseLogger = await HouseLogger.findByPk(req.body.id, {
			include: ['floor'],
		})
		const data = await HouseLogger.destroy({
			where: { id: req.body.id },
			transaction: t,
		})
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
		if (io && houseLogger?.floor?.houseId)
			io.sockets.emit(
				`house-${houseLogger.floor.houseId}`,
				'loggerDelete'
			)
		return success(res, 'Data deleted successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}
