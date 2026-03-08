import { lazy, Suspense, useEffect, useState } from 'react'
import type { GridSortModel, GridFilterModel } from '@mui/x-data-grid'
import { Await, redirect, useLoaderData, data } from 'react-router'
const UserRolesTable = lazy(() => import('./components/UserRolesTable'))
import { Container, useMediaQuery, useTheme } from '@mui/material'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { store } from '../../store/store'
import { adminApi } from '../../store/api/adminApi'
import { showAlert } from '../../store/application-store'
import type { AdminRoleUserClass } from '../Admin/scripts/AdminRoleUserClass'
import type { UserClass } from './scripts/UserClass'

export default function UserRolesView() {
	const { roles } = useLoaderData() as { roles: Promise<AdminRoleUserClass[]> }

	const [sortModel, setSortModel] = useState<GridSortModel>([])
	const [filterModel, setFilterModel] = useState<GridFilterModel>({ items: [] })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		const savedSortModel = localStorage.getItem('userRolesTableSortModel')
		const savedFilterModel = localStorage.getItem('userRolesTableFilterModel')

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
			<Await resolve={roles}>
				{rolesData => (
					<Container maxWidth={isMobile ? 'sm' : 'xl'}>
						<UserRolesTable rolesData={rolesData} initSort={sortModel} initFilter={filterModel} />
					</Container>
				)}
			</Await>
		</Suspense>
	)
}

export async function loader(): Promise<Response | { roles: AdminRoleUserClass[] }> {
	const user = store.getState().account.user as UserClass
	if (!user) {
		store.dispatch(showAlert({ message: 'Unknown user - token not found', severity: 'error' }))
		return redirect('/login')
	}
	try {
		const promise = await store.dispatch(adminApi.endpoints.getAdminRoleUsers.initiate({ userId: user.id })).unwrap()
		if (!promise) {
			throw data('Data not Found', { status: 404 })
		}
		return { roles: promise }
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
