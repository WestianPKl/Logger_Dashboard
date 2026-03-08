import express from 'express'
import {
	getProcessTypes,
	getProcessType,
	addProcessType,
	updateProcessType,
	deleteProcessType,
	getProcessDefinitions,
	getProcessDefinition,
	addProcessDefinition,
	updateProcessDefinition,
	deleteProcessDefinition,
} from '../controller/process.controller.js'
import {
	dataName,
	processProcessTypeId,
} from '../../middleware/body-validation.js'
import validateToken from '../../middleware/jwtValidation.js'

const router = express.Router()

router.post('/process-types', validateToken, getProcessTypes)
router.get('/process-type/:processTypeId', validateToken, getProcessType)
router.post('/process-type', validateToken, [dataName], addProcessType)
router.patch(
	'/process-type/:processTypeId',
	validateToken,
	[dataName],
	updateProcessType
)
router.delete('/process-type/:processTypeId', validateToken, deleteProcessType)

router.post('/process-definitions', validateToken, getProcessDefinitions)
router.get(
	'/process-definition/:processDefinitionId',
	validateToken,
	getProcessDefinition
)
router.post(
	'/process-definition',
	validateToken,
	[dataName, processProcessTypeId],
	addProcessDefinition
)
router.patch(
	'/process-definition/:processDefinitionId',
	validateToken,
	[dataName, processProcessTypeId],
	updateProcessDefinition
)
router.delete(
	'/process-definition/:processDefinitionId',
	validateToken,
	deleteProcessDefinition
)

export default router
