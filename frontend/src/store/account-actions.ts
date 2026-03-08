import type { AppThunk } from './store'
import type { IDecodedToken } from '../modules/Admin/scripts/PermissionInterface'
import { jwtDecode } from 'jwt-decode'
import { accountSlice, getAuthDuration, getUserAvatar, logIn, selectDuration } from './account-store'
import { redirect } from 'react-router'
import type { UserLoginClass } from '../modules/User/scripts/UserLoginClass'
import { showAlert } from './application-store'
import { userApi } from './api/userApi'

export const getAuthTokenDuration = (): AppThunk<number | undefined> => dispatch => {
	const permissionsToken = localStorage.getItem('permissionToken')
	if (!permissionsToken) return undefined

	const decoded: IDecodedToken = jwtDecode(permissionsToken)
	const expiration = new Date(decoded.expiration).getTime()
	const now = Date.now()
	const duration = expiration - now

	if (duration > 0) {
		dispatch(getAuthDuration(duration))
		return duration
	}
	return undefined
}

export const loginAction =
	(data: UserLoginClass): AppThunk =>
	dispatch => {
		const { token, permissionToken } = data
		if (token && permissionToken) {
			const decoded: IDecodedToken = jwtDecode(permissionToken)
			const { id, username, email, createdAt, updatedAt, confirmed } = decoded.user
			const user = { id, username, email, createdAt, updatedAt, confirmed }
			const expiration = Number(decoded.expiration)
			dispatch(logIn({ expiration, user, token }))
		}
	}

export const logoutAction = (): AppThunk => dispatch => {
	localStorage.removeItem('token')
	localStorage.removeItem('permissionToken')
	dispatch(accountSlice.actions.logOff())
}

export const initStore = (): AppThunk<number | undefined> => dispatch => {
	const getAuthDuration = dispatch(getAuthTokenDuration())
	const token = localStorage.getItem('token')
	const permissionToken = localStorage.getItem('permissionToken')

	if (!token || !permissionToken || typeof getAuthDuration !== 'number' || getAuthDuration <= 0) {
		dispatch(logoutAction())
		return undefined
	}

	const decoded: IDecodedToken = jwtDecode(permissionToken)
	dispatch(loginAction({ token, permissionToken }))
	return decoded.user.id
}

export const getToken =
	(): AppThunk<boolean> =>
	(_, getState): boolean => {
		const token = localStorage.getItem('token')
		if (!token) return false

		const tokenDuration = selectDuration(getState())
		if (tokenDuration && tokenDuration > 0) {
			return true
		}
		return false
	}

export const getUserProfile =
	(userId: number): AppThunk =>
	async dispatch => {
		try {
			const userAccount = await dispatch(userApi.endpoints.getUser.initiate(userId)).unwrap()
			dispatch(getUserAvatar(userAccount.avatar))
		} catch (err: any) {
			dispatch(showAlert({ message: err.message, severity: 'error' }))
		}
	}

export function isTokenValid(): boolean {
	const token = localStorage.getItem('token')
	const permissionToken = localStorage.getItem('permissionToken')

	if (!token || !permissionToken) return false

	try {
		const decoded: IDecodedToken = jwtDecode(permissionToken)
		const expiration = new Date(decoded.expiration).getTime()
		return expiration > Date.now()
	} catch {
		return false
	}
}

export function checkAuthLoader(): Response | null {
	if (!isTokenValid()) {
		localStorage.removeItem('token')
		localStorage.removeItem('permissionToken')
		return redirect('/login')
	}
	return null
}

export function checkNotAuthLoader(): Response | null {
	return isTokenValid() ? redirect('/') : null
}
