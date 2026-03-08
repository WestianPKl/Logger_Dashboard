import { useState, useEffect, lazy, Suspense, useMemo } from 'react'
const UserPermissionTable = lazy(() => import('../../User/components/UserPermissionTable'))
import { Container, useMediaQuery, useTheme } from '@mui/material'
import { useGetPermissionsQuery } from '../../../store/api/adminApi'
import type { PermissionClass } from '../scripts/PermissionClass'
import { useAppDispatch } from '../../../store/hooks'
import { showAlert } from '../../../store/application-store'
import LoadingCircle from '../../../components/UI/LoadingCircle'
import type { GridFilterModel, GridSortModel } from '@mui/x-data-grid'

export default function AdminUserPermissionTable({
	userId,
	isAdmin,
	roleId,
}: {
	userId?: number
	isAdmin?: boolean
	roleId?: number
}) {
	const [permissionData, setPermissionData] = useState<PermissionClass[]>([])

	const dispatch = useAppDispatch()

	const params = useMemo(() => {
		if (userId) return { userId }
		if (roleId) return { roleId }
		return undefined
	}, [userId, roleId])

	const skip = !params

	const { data, error } = useGetPermissionsQuery(params, { skip })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	const [sortModel, setSortModel] = useState<GridSortModel>([])
	const [filterModel, setFilterModel] = useState<GridFilterModel>({ items: [] })

	useEffect(() => {
		if (data) {
			setPermissionData(data)
		}
		if (error) {
			const message = (error as any)?.data?.message || (error as any)?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [dispatch, error])

	useEffect(() => {
		const savedSortModel = localStorage.getItem('adminUserPermissionTableSortModel')
		const savedFilterModel = localStorage.getItem('adminUserPermissionTableFilterModel')

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
				<UserPermissionTable
					permissionData={permissionData}
					isAdmin={isAdmin}
					userId={userId}
					roleId={roleId}
					initSort={sortModel}
					initFilter={filterModel}
				/>
			</Suspense>
		</Container>
	)
}
