import express from 'express'
import {
	getAdmFunctionalityDefinitions,
	getAdmFunctionalityDefinition,
	addAdmFunctionalityDefinition,
	updateAdmFunctionalityDefinition,
	deleteAdmFunctionalityDefinition,
	getAdmObjectDefinitions,
	getAdmObjectDefinition,
	addAdmObjectDefinition,
	updateAdmObjectDefinition,
	deleteAdmObjectDefinition,
	getAdmAccessLevelDefinitions,
	getAdmAccessLevelDefinition,
	addAdmAccessLevelDefinition,
	updateAdmAccessLevelDefinition,
	deleteAdmAccessLevelDefinition,
	getAdmRoles,
	getAdmRole,
	addAdmRole,
	updateAdmRole,
	deleteAdmRole,
	getAdmRoleUsers,
	addAdmRoleUser,
	addAdmPermission,
	deleteAdmRoleUser,
	deleteAdmPermission,
} from '../controller/adm.controller.js'
import { getPermissions } from '../controller/permission.controller.js'
import {
	dataName,
	dataDescription,
	admAccessLevel,
	admUserId,
	admRoleId,
	admFunctionalityDefinitionId,
	admAccessLevelDefinitionId,
} from '../../middleware/body-validation.js'
import validateToken from '../../middleware/jwtValidation.js'

const router = express.Router()

router.post('/adm-permissions', validateToken, getPermissions)
router.post(
	'/adm-permission',
	validateToken,
	[admFunctionalityDefinitionId, admAccessLevelDefinitionId],
	addAdmPermission
)
router.delete(
	'/adm-permission/:admPermissionId',
	validateToken,
	deleteAdmPermission
)
router.post(
	'/adm-functionality-definitions',
	validateToken,
	getAdmFunctionalityDefinitions
)
router.get(
	'/adm-functionality-definition/:admFunctionalityDefinitionId',
	validateToken,
	getAdmFunctionalityDefinition
)
router.post(
	'/adm-functionality-definition',
	validateToken,
	[dataName, dataDescription],
	addAdmFunctionalityDefinition
)
router.patch(
	'/adm-functionality-definition/:admFunctionalityDefinitionId',
	validateToken,
	[dataName, dataDescription],
	updateAdmFunctionalityDefinition
)
router.delete(
	'/adm-functionality-definition/:admFunctionalityDefinitionId',
	validateToken,
	deleteAdmFunctionalityDefinition
)
router.post('/adm-object-definitions', validateToken, getAdmObjectDefinitions)
router.get(
	'/adm-object-definition/:admObjectDefinitionId',
	validateToken,
	getAdmObjectDefinition
)
router.post(
	'/adm-object-definition',
	validateToken,
	[dataName, dataDescription],
	addAdmObjectDefinition
)
router.patch(
	'/adm-object-definition/:admObjectDefinitionId',
	validateToken,
	[dataName, dataDescription],
	updateAdmObjectDefinition
)
router.delete(
	'/adm-object-definition/:admObjectDefinitionId',
	validateToken,
	deleteAdmObjectDefinition
)
router.post(
	'/adm-access-level-definitions',
	validateToken,
	getAdmAccessLevelDefinitions
)
router.get(
	'/adm-access-level-definition/:admAccessLevelDefinitionId',
	validateToken,
	getAdmAccessLevelDefinition
)
router.post(
	'/adm-access-level-definition',
	validateToken,
	[dataName, admAccessLevel],
	addAdmAccessLevelDefinition
)
router.patch(
	'/adm-access-level-definition/:admAccessLevelDefinitionId',
	validateToken,
	[dataName, admAccessLevel],
	updateAdmAccessLevelDefinition
)
router.delete(
	'/adm-access-level-definition/:admAccessLevelDefinitionId',
	validateToken,
	deleteAdmAccessLevelDefinition
)
router.post('/adm-roles', validateToken, getAdmRoles)
router.get('/adm-role/:admRoleId', validateToken, getAdmRole)
router.post('/adm-role', validateToken, [dataName, dataDescription], addAdmRole)
router.patch(
	'/adm-role/:admRoleId',
	validateToken,
	[dataName, dataDescription],
	updateAdmRole
)
router.delete('/adm-role/:admRoleId', validateToken, deleteAdmRole)
router.post('/adm-role-users', validateToken, getAdmRoleUsers)
router.post(
	'/adm-role-user',
	validateToken,
	[admRoleId, admUserId],
	addAdmRoleUser
)
router.delete('/adm-role-user/:admRoleId', validateToken, deleteAdmRoleUser)

export default router
