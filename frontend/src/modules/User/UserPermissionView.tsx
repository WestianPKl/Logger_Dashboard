import { lazy, Suspense, useEffect, useState } from 'react'
import type { GridSortModel, GridFilterModel } from '@mui/x-data-grid'
const UserPermissionTable = lazy(() => import('./components/UserPermissionTable'))
import { Container, useMediaQuery, useTheme } from '@mui/material'
import { adminApi } from '../../store/api/adminApi'
import type { PermissionClass } from '../Admin/scripts/PermissionClass'
import { showAlert } from '../../store/application-store'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { Await, redirect, useLoaderData, data } from 'react-router'
import { store } from '../../store/store'
import type { UserClass } from './scripts/UserClass'

export default function UserPermissionView() {
	const { permissionData, userId } = useLoaderData() as { permissionData: Promise<PermissionClass[]>; userId: number }
	const [sortModel, setSortModel] = useState<GridSortModel>([])
	const [filterModel, setFilterModel] = useState<GridFilterModel>({ items: [] })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		const savedSortModel = localStorage.getItem('userPermissionTableSortModel')
		const savedFilterModel = localStorage.getItem('userPermissionTableFilterModel')

		if (savedFilterModel) {
			try {
				const parsedFilterModel = JSON.parse(savedFilterModel)
				if (parsedFilterModel && typeof parsedFilterModel === 'object' && Array.isArray(parsedFilterModel.items)) {
					setFilterModel(parsedFilterModel)
				}
			} catch (err) {
				// pass
			}
		}

		if (savedSortModel) {
			try {
				const parsedSortModel = JSON.parse(savedSortModel)
				if (Array.isArray(parsedSortModel)) {
					setSortModel(parsedSortModel)
				}
			} catch (err) {
				// pass
			}
		}
	}, [])

	return (
		<Suspense fallback={<LoadingCircle />}>
			<Await resolve={permissionData}>
				{permissions => (
					<Container maxWidth={isMobile ? 'sm' : 'xl'}>
						<UserPermissionTable
							permissionData={permissions}
							userId={userId}
							initSort={sortModel}
							initFilter={filterModel}
						/>
					</Container>
				)}
			</Await>
		</Suspense>
	)
}

export async function loader(): Promise<Response | { permissionData: PermissionClass[]; userId: number }> {
	if (!(store.getState().account.user as UserClass)) {
		store.dispatch(showAlert({ message: 'Unknown user - token not found', severity: 'error' }))
		return redirect('/login')
	}
	try {
		const userId = (store.getState().account.user as UserClass).id!
		const promise = await store.dispatch(adminApi.endpoints.getPermissions.initiate({ userId })).unwrap()
		if (!promise) {
			throw data('Data not Found', { status: 404 })
		}
		return { permissionData: promise, userId: userId }
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
