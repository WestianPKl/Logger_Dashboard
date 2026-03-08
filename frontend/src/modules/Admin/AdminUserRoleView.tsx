import { lazy, Suspense, useEffect, useState } from 'react'
const AdminRoleUserTable = lazy(() => import('./components/AdminRoleUserTable'))
import { Container, useMediaQuery, useTheme } from '@mui/material'
import { adminApi } from '../../store/api/adminApi'
import { AdminRoleClass } from '../Admin/scripts/AdminRoleClass'
import { showAlert } from '../../store/application-store'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { store } from '../../store/store'
import { data, useLoaderData, Await, type LoaderFunctionArgs } from 'react-router'
import type { GridFilterModel, GridSortModel } from '@mui/x-data-grid'

export default function AdminUserRoleView() {
	const { adminRoles, roleId } = useLoaderData() as { adminRoles: AdminRoleClass; roleId: number }

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	const [sortModel, setSortModel] = useState<GridSortModel>([])
	const [filterModel, setFilterModel] = useState<GridFilterModel>({ items: [] })

	useEffect(() => {
		const savedSortModel = localStorage.getItem('adminRoleUserTableSortModel')
		const savedFilterModel = localStorage.getItem('adminRoleUserTableFilterModel')

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
				<Await resolve={adminRoles}>
					{adminRolesData => (
						<AdminRoleUserTable
							usersData={adminRolesData.users}
							roleId={roleId}
							isAdmin={true}
							initSort={sortModel}
							initFilter={filterModel}
						/>
					)}
				</Await>
			</Suspense>
		</Container>
	)
}

export async function loader({ params }: LoaderFunctionArgs): Promise<{ adminRoles: AdminRoleClass; roleId: number }> {
	const roleId = params.roleId
	if (!roleId) {
		throw data('No role Id', { status: 400 })
	}
	try {
		const promise = await store.dispatch(adminApi.endpoints.getAdminRoles.initiate({ roleId })).unwrap()
		if (!promise) {
			throw data('Data not Found', { status: 404 })
		}
		return {
			adminRoles: promise[0],
			roleId: parseInt(roleId),
		}
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
