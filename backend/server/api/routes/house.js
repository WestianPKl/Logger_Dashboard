import express from 'express'
import {
	getHouses,
	getHouse,
	addHouse,
	updateHouse,
	deleteHouse,
	getHouseFloors,
	getHouseFloor,
	addHouseFloor,
	updateHouseFloor,
	updateHouseFloorLayout,
	deleteHouseFloor,
	getHouseLoggers,
	getHouseLogger,
	addHouseLogger,
	updateHouseLogger,
	deleteHouseLogger,
} from '../controller/house.controller.js'
import {
	dataName,
	houseEquLogger,
	houseNumber,
	houseHouseId,
} from '../../middleware/body-validation.js'
import { imageUpload } from '../../middleware/file.js'
import validateToken from '../../middleware/jwtValidation.js'

const router = express.Router()

router.post('/houses', validateToken, getHouses)
router.get('/house/:houseId', validateToken, getHouse)
router.post(
	'/house',
	validateToken,
	imageUpload.single('pictureLink'),
	[dataName, houseNumber],
	addHouse
)
router.patch(
	'/house/:houseId',
	validateToken,
	imageUpload.single('pictureLink'),
	[dataName, houseNumber],
	updateHouse
)
router.delete('/house/:houseId', validateToken, deleteHouse)
router.post('/house-floors', validateToken, getHouseFloors)
router.get('/house-floor/:houseFloorId', validateToken, getHouseFloor)
router.post(
	'/house-floor',
	validateToken,
	imageUpload.single('layout'),
	[dataName, houseHouseId],
	addHouseFloor
)
router.patch(
	'/house-floor/:houseFloorId',
	validateToken,
	imageUpload.single('layout'),
	[dataName, houseHouseId],
	updateHouseFloor
)
router.patch(
	'/house-floor-layout/:houseFloorId',
	validateToken,
	updateHouseFloorLayout
)
router.delete('/house-floor/:houseFloorId', validateToken, deleteHouseFloor)
router.post('/house-loggers', validateToken, getHouseLoggers)
router.get('/house-logger/:houseLoggerId', validateToken, getHouseLogger)
router.post('/house-logger', validateToken, [houseEquLogger], addHouseLogger)
router.patch(
	'/house-logger/:houseLoggerId',
	validateToken,
	[houseEquLogger],
	updateHouseLogger
)
router.delete('/house-logger/:houseLoggerId', validateToken, deleteHouseLogger)

export default router
