import type { AppThunk } from './store'
import { selectPermissions, selectAccessLevels } from './auth-store'
import { getPermissions, getAccessLevels } from './auth-store'
import { showAlert } from './application-store'
import { adminApi } from './api/adminApi'
import type { RootState } from './store'

export const fetchPermission =
	(userId: number): AppThunk =>
	async dispatch => {
		try {
			const permissions = await dispatch(adminApi.endpoints.getPermissions.initiate({ userId })).unwrap()
			dispatch(getPermissions(permissions))
		} catch (err: any) {
			dispatch(showAlert({ message: err.message, severity: 'error' }))
		}
	}

export const fetchAccessLevels = (): AppThunk => async dispatch => {
	try {
		const accessLevels = await dispatch(adminApi.endpoints.getAccessLevelDefinitions.initiate({})).unwrap()
		dispatch(getAccessLevels(accessLevels))
	} catch (err: any) {
		dispatch(showAlert({ message: err.message, severity: 'error' }))
	}
}

export function checkPermission(
	functionName: string,
	objectName: string | null,
	requestedAccessLevel: string,
): (state: RootState) => boolean {
	return (state: RootState): boolean => {
		let returnValue = false
		if (!requestedAccessLevel) return returnValue
		const permissions = selectPermissions(state)
		const accessLevels = selectAccessLevels(state)
		if (permissions.length > 0) {
			let permissionItems = permissions.filter(_ => true)
			const requestedAccessLevelsData = accessLevels.filter(e => e.name === requestedAccessLevel)
			if (
				!requestedAccessLevelsData ||
				requestedAccessLevelsData.length < 1 ||
				!requestedAccessLevelsData[0].accessLevel
			) {
				return false
			}
			const requestedAccessValue = requestedAccessLevelsData[0].accessLevel
			permissionItems = permissionItems.filter(item => item.accessLevelDefinition?.accessLevel! >= requestedAccessValue)
			if (functionName == null) {
				permissionItems = permissionItems.filter(item => item.admFunctionalityDefinitionId === null)
			} else {
				permissionItems = permissionItems.filter(
					item => item.functionalityDefinition && item.functionalityDefinition.name === functionName,
				)
			}
			if (objectName == null) {
				permissionItems = permissionItems.filter(item => item.admObjectDefinitionId === undefined)
			} else {
				permissionItems = permissionItems.filter(
					item => item.objectDefinition && item.objectDefinition.name === objectName,
				)
			}
			if (permissionItems.length > 0) {
				returnValue = true
			}
		}
		return returnValue
	}
}

export const canRead = (functionName: string, objectName: string | null) => (state: RootState) =>
	checkPermission(functionName, objectName, 'READ')(state)

export const canWrite = (functionName: string, objectName: string | null) => (state: RootState) =>
	checkPermission(functionName, objectName, 'WRITE')(state)

export const canDelete = (functionName: string, objectName: string | null) => (state: RootState) =>
	checkPermission(functionName, objectName, 'DELETE')(state)
