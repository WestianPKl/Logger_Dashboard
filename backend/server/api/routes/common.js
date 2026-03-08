import express from 'express'
import {
	getErrorLogs,
	getErrorLog,
	addErrorLog,
	updateErrorLog,
	deleteErrorLog,
} from '../controller/common.controller.js'
import {
	errorMessage,
	errorType,
	errorSeverity,
} from '../../middleware/body-validation.js'

const router = express.Router()

router.post('/error-logs', getErrorLogs)
router.get('/error-log/:errorLogId', getErrorLog)
router.post('/error-log', [errorMessage, errorType, errorSeverity], addErrorLog)
router.patch(
	'/error-log/:errorLogId',
	[errorMessage, errorType, errorSeverity],
	updateErrorLog
)
router.delete('/error-log/:errorLogId', deleteErrorLog)

export default router
