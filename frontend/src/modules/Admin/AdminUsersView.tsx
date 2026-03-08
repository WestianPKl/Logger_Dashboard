import { lazy, Suspense, useEffect, useState } from 'react'
import type { GridSortModel, GridFilterModel } from '@mui/x-data-grid'
const AdminUserTable = lazy(() => import('./components/AdminUserTable'))
import type { UserClass } from '../User/scripts/UserClass'
import { Container, useMediaQuery, useTheme } from '@mui/material'
import { userApi } from '../../store/api/userApi'
import { showAlert } from '../../store/application-store'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { store } from '../../store/store'
import { data, useLoaderData, Await } from 'react-router'

export default function AdminUsersView() {
	const { users } = useLoaderData() as { users: UserClass[] }

	const [sortModel, setSortModel] = useState<GridSortModel>([])
	const [filterModel, setFilterModel] = useState<GridFilterModel>({ items: [] })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		const savedSortModel = localStorage.getItem('adminUserTableSortModel')
		const savedFilterModel = localStorage.getItem('adminUserTableFilterModel')

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
		<Container maxWidth={isMobile ? 'sm' : 'xl'}>
			<Suspense fallback={<LoadingCircle />}>
				<Await resolve={users}>
					{usersData => <AdminUserTable users={usersData} initSort={sortModel} initFilter={filterModel} />}
				</Await>
			</Suspense>
		</Container>
	)
}

export async function loader(): Promise<{ users: UserClass[] }> {
	try {
		const promise = await store.dispatch(userApi.endpoints.getUsers.initiate({})).unwrap()
		if (!promise) {
			throw data('Data not Found', { status: 404 })
		}
		return { users: promise }
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
