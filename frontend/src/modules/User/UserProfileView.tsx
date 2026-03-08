import { lazy, Suspense } from 'react'
import { Await, redirect, useLoaderData, data } from 'react-router'
import { Container, useMediaQuery, useTheme } from '@mui/material'
const UserProfile = lazy(() => import('./components/UserProfile'))
import { userApi } from '../../store/api/userApi'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { store } from '../../store/store'
import { showAlert } from '../../store/application-store'
import type { UserClass } from './scripts/UserClass'

export default function UserProfileView() {
	const { userData } = useLoaderData() as { userData: Promise<UserClass> }
	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	return (
		<Suspense fallback={<LoadingCircle />}>
			<Await resolve={userData}>
				{user => (
					<Container maxWidth={isMobile ? 'sm' : 'xl'} sx={{ textAlign: 'center' }}>
						<UserProfile user={user} />
					</Container>
				)}
			</Await>
		</Suspense>
	)
}

export async function loader(): Promise<Response | { userData: UserClass }> {
	if (!(store.getState().account.user as UserClass)) {
		store.dispatch(showAlert({ message: 'Unknown user - token not found', severity: 'error' }))
		return redirect('/login')
	}
	try {
		const userId = (store.getState().account.user as UserClass).id!
		const promise = await store.dispatch(userApi.endpoints.getUser.initiate(userId)).unwrap()
		if (!promise) {
			throw data('Data not Found', { status: 404 })
		}
		return { userData: promise }
	} catch (err: any) {
		store.dispatch(
			showAlert({
				message: err?.data?.message || err?.message || 'Something went wrong!',
				severity: 'error',
			}),
		)
		throw err
	}
}
