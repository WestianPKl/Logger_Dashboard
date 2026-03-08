import { useState, useEffect, lazy, Suspense } from 'react'
const AdminRoleUserTable = lazy(() => import('./AdminRoleUserTable'))
import { Container, useMediaQuery, useTheme } from '@mui/material'
import { useGetAdminRolesQuery } from '../../../store/api/adminApi'
import { AdminRoleClass } from '../scripts/AdminRoleClass'
import { useAppDispatch } from '../../../store/hooks'
import { showAlert } from '../../../store/application-store'
import LoadingCircle from '../../../components/UI/LoadingCircle'
import type { GridFilterModel, GridSortModel } from '@mui/x-data-grid'

export default function AdminUserRoleMain({ isAdmin, roleId }: { isAdmin?: boolean; roleId?: number }) {
	const [rolesData, setRolesData] = useState<AdminRoleClass>(new AdminRoleClass())

	const dispatch = useAppDispatch()

	const { data, error } = useGetAdminRolesQuery({ id: roleId })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	const [sortModel, setSortModel] = useState<GridSortModel>([])
	const [filterModel, setFilterModel] = useState<GridFilterModel>({ items: [] })

	useEffect(() => {
		if (data) {
			setRolesData(data[0])
		}
		if (error) {
			const message = (error as any)?.data?.message || (error as any)?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [dispatch, data, error])

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
				<AdminRoleUserTable
					usersData={rolesData.users}
					roleId={roleId}
					isAdmin={isAdmin}
					initSort={sortModel}
					initFilter={filterModel}
				/>
			</Suspense>
		</Container>
	)
}
