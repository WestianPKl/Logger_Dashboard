import { useEffect, useRef } from 'react'
import { RouterProvider } from 'react-router'
import { router } from './router/routes'
import SnackBar from './components/UI/SnackBar'
import { selectTimeout, selectIsActive, selectMessage, selectSeverity, hideAlert } from './store/application-store'
import { useAppDispatch, useAppSelector } from './store/hooks'

export default function App() {
	const dispatch = useAppDispatch()
	const isActive = useAppSelector(selectIsActive)
	const timeout = useAppSelector(selectTimeout)
	const message = useAppSelector(selectMessage)
	const severity = useAppSelector(selectSeverity)
	const alertTimeout = useRef<ReturnType<typeof setTimeout> | null>(null)

	useEffect(() => {
		if (isActive) {
			if (alertTimeout.current) clearTimeout(alertTimeout.current)
			alertTimeout.current = setTimeout(() => {
				dispatch(hideAlert())
			}, timeout)
		}
		return () => {
			if (alertTimeout.current) clearTimeout(alertTimeout.current)
		}
	}, [isActive, timeout, dispatch])

	return (
		<>
			<RouterProvider router={router} />
			{isActive && <SnackBar message={message} severity={severity} />}
		</>
	)
}
