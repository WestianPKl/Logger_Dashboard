import { Sequelize } from 'sequelize'
import { verifyToken } from '../../libs/jwtToken.js'
import User from '../model/users/user.model.js'
import SuperUsers from '../model/users/superusers.model.js'
import AdmViewPermissions from '../model/adm/admViewPermission.model.js'
import AdmRolesUser from '../model/adm/admRolesUser.model.js'
import FunctionalityDefinition from '../model/adm/admFunctionalityDefinition.model.js'
import ObjectDefinition from '../model/adm/admObjectDefinition.model.js'
import AccessLevelDefinition from '../model/adm/admAccessLevelDefinitions.model.js'
import {
	badRequest,
	success,
	internalServerError,
} from '../../util/responseHelper.js'

const Op = Sequelize.Op

export async function getPermissions(req, res) {
	try {
		const queryObject = req.body
		let data = null
		if (queryObject.userId) {
			data = await getUserAllPermission(queryObject.userId)
		} else if (queryObject.roleId) {
			data = await getRoleAllPermission(queryObject.roleId)
		} else {
			return badRequest(res, 'No permissions', data)
		}
		return success(res, 'Success', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

async function getRoleAllPermission(roleId) {
	if (!roleId) throw new Error('Error - role must be set')
	let permission = []
	const userPermission = await AdmViewPermissions.findAll({
		where: { roleId },
		include: [
			'functionalityDefinition',
			'objectDefinition',
			'accessLevelDefinition',
		],
	})
	if (userPermission) {
		permission = [...userPermission]
	}
	return permission
}

async function getUserAllPermission(userId) {
	if (!userId) throw new Error('Error - user must be set')
	let permission = []
	const userPermission = await AdmViewPermissions.findAll({
		where: { userId },
		include: [
			'functionalityDefinition',
			'objectDefinition',
			'accessLevelDefinition',
		],
	})
	if (userPermission) {
		permission = [...userPermission]
	}

	const admUserRoles = await AdmRolesUser.findAll({ where: { userId } })
	const userRolesArray = admUserRoles.map((item) => item.roleId)

	if (userRolesArray && userRolesArray.length > 0) {
		const rolePermissionQuery = { roleId: { [Op.in]: userRolesArray } }
		const rolePermission = await AdmViewPermissions.findAll({
			where: rolePermissionQuery,
			include: [
				'functionalityDefinition',
				'objectDefinition',
				'accessLevelDefinition',
			],
		})
		if (rolePermission) {
			permission = [...permission, ...rolePermission]
		}
	}

	const user = await User.findOne({ where: { id: userId } })
	const superuser = await SuperUsers.findOne({ where: { userId: user.id } })
	if (superuser) {
		permission.push({
			id: -1,
			userId,
			roleId: null,
			functionalityDefinitionId: null,
			accessLevelDefinitionId: 5,
			accessLevelDefinition: {
				id: 5,
				name: 'Admin',
				accessLevel: 50,
			},
		})
	}
	return permission
}

async function getUserPermissions(
	userId,
	functionalityId,
	objectId,
	requestedAccessLevel
) {
	if (!userId) throw new Error('Error - user must be set')
	let permissions = await getUserAllPermission(userId)
	if (!permissions) return []
	permissions = permissions.filter(
		(item) =>
			item.accessLevelDefinition.accessLevel >=
			requestedAccessLevel.accessLevel
	)

	if (functionalityId !== undefined) {
		permissions = permissions.filter(
			(item) => item.functionalityDefinitionId == functionalityId
		)
	}
	if (objectId !== undefined && objectId !== null) {
		permissions = permissions.filter(
			(item) => item.objectDefinitionId == objectId
		)
	}
	return permissions
}

export async function checkPermission(
	req,
	functionalityName,
	objectName,
	requestedAccessLevel
) {
	const authHeader = req.get('Authorization')
	const token = authHeader && authHeader.split(' ')[1]
	if (!token) return false

	const tokenDecoded = verifyToken(token)
	if (!tokenDecoded) return false

	const user = tokenDecoded.user
	if (!user) return false

	const functionality = await FunctionalityDefinition.findOne({
		where: { name: functionalityName },
	})
	if (!functionality) return false

	let object = null
	if (objectName !== null && objectName !== undefined) {
		object = await ObjectDefinition.findOne({
			where: { name: objectName },
		})
		if (!object) return false
	}
	const accessLevel = await AccessLevelDefinition.findOne({
		where: { name: requestedAccessLevel },
	})
	if (!accessLevel) return false

	const userPermissions = await getUserPermissions(
		user.id,
		functionality.id,
		object ? object.id : null,
		accessLevel
	)
	const superuser = await SuperUsers.findOne({ where: { userId: user.id } })
	if (superuser) return true

	if (!userPermissions || userPermissions.length === 0) {
		return false
	}
	return true
}
