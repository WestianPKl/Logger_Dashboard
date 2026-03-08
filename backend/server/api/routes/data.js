import express from 'express'
import {
	getDataDefinitions,
	getDataDefinition,
	addDataDefinition,
	updateDataDefinition,
	deleteDataDefinition,
	getDataLogs,
	getDataLog,
	addDataLog,
	updateDataLog,
	deleteDataLog,
	getDataLastValues,
	getDataLastValue,
	addDataLastValue,
	updateDataLastValue,
	deleteDataLastValue,
	getDataLastValuesView,
	getDataConnectedSensorView,
	getDataToken,
	getDataLogsView,
	getLastValuesForLoggers,
} from '../controller/data.controller.js'
import {
	dataName,
	dataUnit,
	dataValue,
	dataDescription,
	equSensor,
	houseEquLogger,
	dataDataDefinitionId,
	dataDataLogId,
} from '../../middleware/body-validation.js'
import validateToken from '../../middleware/jwtValidation.js'

const router = express.Router()

router.post('/data-definitions', validateToken, getDataDefinitions)
router.get(
	'/data-definition/:dataDefinitionId',
	validateToken,
	getDataDefinition
)
router.post(
	'/data-definition',
	validateToken,
	[dataName, dataUnit, dataDescription],
	addDataDefinition
)
router.patch(
	'/data-definition/:dataDefinitionId',
	validateToken,
	[dataName, dataUnit, dataDescription],
	updateDataDefinition
)
router.delete(
	'/data-definition/:dataDefinitionId',
	validateToken,
	deleteDataDefinition
)
router.post('/data-logs', validateToken, getDataLogs)
router.get('/data-log/:dataLogId', validateToken, getDataLog)
router.post('/data-log', validateToken, addDataLog)
router.patch(
	'/data-log/:dataLogId',
	validateToken,
	[dataValue, dataDataDefinitionId, houseEquLogger, equSensor],
	updateDataLog
)
router.delete('/data-log/:dataLogId', validateToken, deleteDataLog)
router.post('/data-last-values', validateToken, getDataLastValues)
router.get('/data-last-value/:dataLastValueId', validateToken, getDataLastValue)
router.post(
	'/data-last-value',
	validateToken,
	[dataDataLogId],
	addDataLastValue
)
router.patch(
	'/data-last-value/:dataLastValueId',
	validateToken,
	[dataDataLogId],
	updateDataLastValue
)
router.delete(
	'/data-last-value/:dataLastValueId',
	validateToken,
	deleteDataLastValue
)
router.post('/data-last-values-view', validateToken, getDataLastValuesView)
router.post(
	'/data-connected-sensor-view',
	validateToken,
	getDataConnectedSensorView
)
router.post('/data-logs-view', validateToken, getDataLogsView)
router.post('/data-last-values-multi', getLastValuesForLoggers)
router.get('/data-token', getDataToken)

export default router
