import { useEffect, useCallback } from 'react'
import { Outlet, redirect, useLoaderData, useSubmit } from 'react-router'
import AppBarView from './AppBarView'
import Wrapper from '../../components/UI/Wrapper'
import { Box } from '@mui/material'
import { useAppDispatch } from '../../store/hooks'
import {
	initStore,
	getAuthTokenDuration,
	getUserProfile,
	logoutAction,
	isTokenValid,
} from '../../store/account-actions'
import { store } from '../../store/store'
import { fetchAccessLevels, fetchPermission } from '../../store/auth-actions'

const TOKEN_CHECK_INTERVAL = 60000

export default function RootView() {
	const { token } = useLoaderData()
	const submit = useSubmit()
	const dispatch = useAppDispatch()

	const handleLogout = useCallback(() => {
		submit(null, { action: '/logout', method: 'POST' })
	}, [submit])

	useEffect(() => {
		if (!token) return

		let timeoutId: number | undefined
		let intervalId: number | undefined

		const duration = dispatch(getAuthTokenDuration())

		if (typeof duration === 'number' && duration <= 0) {
			handleLogout()
			return
		}

		if (typeof duration === 'number' && duration > 0) {
			timeoutId = window.setTimeout(() => {
				handleLogout()
			}, duration)
		}

		intervalId = window.setInterval(() => {
			if (!isTokenValid()) {
				handleLogout()
			}
		}, TOKEN_CHECK_INTERVAL)

		const handleVisibilityChange = () => {
			if (document.visibilityState === 'visible' && !isTokenValid()) {
				handleLogout()
			}
		}

		const handleFocus = () => {
			if (!isTokenValid()) {
				handleLogout()
			}
		}

		document.addEventListener('visibilitychange', handleVisibilityChange)
		window.addEventListener('focus', handleFocus)

		return () => {
			if (timeoutId !== undefined) clearTimeout(timeoutId)
			if (intervalId !== undefined) clearInterval(intervalId)
			document.removeEventListener('visibilitychange', handleVisibilityChange)
			window.removeEventListener('focus', handleFocus)
		}
	}, [token, dispatch, handleLogout])

	return (
		<Box component={'section'}>
			<AppBarView />
			<Wrapper>
				<Outlet />
			</Wrapper>
		</Box>
	)
}

export async function loader(): Promise<{
	token: string | null
}> {
	const userId = store.dispatch(initStore())
	if (typeof userId === 'number' && localStorage.getItem('token') && localStorage.getItem('permissionToken')) {
		await Promise.all([
			store.dispatch(getUserProfile(userId)),
			store.dispatch(fetchPermission(userId)),
			store.dispatch(fetchAccessLevels()),
		])
	}
	return {
		token: localStorage.getItem('token'),
	}
}

export function action(): Response {
	store.dispatch(logoutAction())
	return redirect('/login')
}
