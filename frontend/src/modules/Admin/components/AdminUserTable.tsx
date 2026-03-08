import { useState, useEffect, useMemo, useCallback } from 'react'
import { Box, Typography, Icon, Button, useMediaQuery, useTheme, IconButton } from '@mui/material'
import SecurityIcon from '@mui/icons-material/Security'
import PermIdentityIcon from '@mui/icons-material/PermIdentity'
import {
	DataGrid,
	type GridColDef,
	type GridFilterModel,
	type GridRowSelectionModel,
	type GridSortModel,
} from '@mui/x-data-grid'
import type { IUserTableProps } from '../scripts/IAdmin'
import type { UserClass } from '../../User/scripts/UserClass'
import AdminUserPermissionDialog from './AdminUserPermissionDialog'
import formatLocalDateTime from '../../../components/scripts/ComponentsInterface'

export default function AdminUserTable({ users, initSort, initFilter }: IUserTableProps) {
	const [selectedItems, setSelectedItems] = useState<UserClass[]>([])
	const [rowSelectionModel, setRowSelectionModel] = useState<GridRowSelectionModel>({
		type: 'include',
		ids: new Set(),
	})
	const [openPermissionDialog, setOpenPermissionDialog] = useState<boolean>(false)
	const [sortModel, setSortModel] = useState<GridSortModel>(initSort)
	const [filterModel, setFilterModel] = useState<GridFilterModel>(initFilter)

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	const usersMap = useMemo(() => {
		const map = new Map()
		users.forEach(item => {
			if (item.id) map.set(item.id, item)
		})
		return map
	}, [users])

	useEffect(() => {
		localStorage.setItem('adminUserTableSortModel', JSON.stringify(sortModel))
	}, [sortModel])

	useEffect(() => {
		localStorage.setItem('adminUserTableFilterModel', JSON.stringify(filterModel))
	}, [filterModel])

	useEffect(() => {
		const selectedIds = [...rowSelectionModel.ids]
		setSelectedItems(selectedIds.map(id => usersMap.get(Number(id))).filter(Boolean))
	}, [rowSelectionModel, usersMap])

	function handleClickPermissionOpen(): void {
		setOpenPermissionDialog(true)
	}
	function handleClosePermission(): void {
		setOpenPermissionDialog(false)
	}

	const handleSortModelChange = useCallback((newSortModel: GridSortModel) => {
		setSortModel(newSortModel)
	}, [])

	const handleRowSelectionModelChange = useCallback((newRowSelectionModel: GridRowSelectionModel) => {
		setRowSelectionModel(newRowSelectionModel)
	}, [])

	return (
		<Box sx={{ textAlign: 'center' }}>
			<Box sx={{ textAlign: 'left' }}>
				<Box sx={{ display: 'flex' }}>
					<Icon sx={{ mr: '0.5rem' }}>
						<PermIdentityIcon />
					</Icon>
					<Typography variant='h6' component='p'>
						Users database
					</Typography>
				</Box>
				<Typography component='span'>Your database containg all the registered users.</Typography>
			</Box>
			<Box sx={{ mt: '2rem' }}>
				<Box sx={{ mb: '1rem', textAlign: 'right' }}>
					{selectedItems.length > 0 && (
						<>
							{!isMobile ? (
								<Button
									sx={{ ml: '0.3rem' }}
									variant='contained'
									color='info'
									type='button'
									size={isMobile ? 'small' : 'medium'}
									onClick={handleClickPermissionOpen}>
									Assign permission
								</Button>
							) : (
								<IconButton
									sx={{ ml: '0.3rem' }}
									color='info'
									type='button'
									size={isMobile ? 'small' : 'medium'}
									onClick={handleClickPermissionOpen}>
									<SecurityIcon />
								</IconButton>
							)}
							<AdminUserPermissionDialog
								handleCloseAdd={handleClosePermission}
								openAddDialog={openPermissionDialog}
								selectedItems={selectedItems}
							/>
						</>
					)}
				</Box>
				<DataGrid
					rows={users}
					columns={useMemo<GridColDef[]>(
						() => [
							{ field: 'id', headerName: 'ID', width: 100 },
							{ field: 'username', headerName: 'Username', width: 360 },
							{ field: 'email', headerName: 'Email', width: 360 },
							{
								field: 'createdAt',
								headerName: 'Creation date',
								width: 160,
								valueGetter: (_, row) => `${formatLocalDateTime(row.createdAt)}`,
							},
							{
								field: 'updatedAt',
								headerName: 'Update date',
								width: 160,
								valueGetter: (_, row) => `${formatLocalDateTime(row.updatedAt)}`,
							},
							{ field: 'avatar', headerName: 'Avatar', width: 360 },
						],
						[],
					)}
					initialState={{ pagination: { paginationModel: { page: 0, pageSize: 15 } } }}
					pageSizeOptions={[15, 30, 45]}
					checkboxSelection={true}
					disableRowSelectionOnClick={true}
					sx={{
						border: 'none',
						borderRadius: 3,
						width: '100%',
						'& .MuiDataGrid-columnHeaders': { backgroundColor: 'rgba(74, 158, 158, 0.06)', fontWeight: 600 },
						'& .MuiDataGrid-row:hover': { backgroundColor: 'rgba(74, 158, 158, 0.04)' },
						'& .MuiDataGrid-cell': { borderColor: 'rgba(0, 0, 0, 0.04)' },
					}}
					density='comfortable'
					disableColumnResize={true}
					disableColumnSelector={true}
					disableVirtualization={true}
					disableMultipleRowSelection={true}
					onRowSelectionModelChange={handleRowSelectionModelChange}
					rowSelectionModel={rowSelectionModel}
					sortModel={sortModel}
					showToolbar
					onSortModelChange={handleSortModelChange}
					filterModel={filterModel}
					onFilterModelChange={newFilterModel => setFilterModel(newFilterModel)}
				/>
			</Box>
		</Box>
	)
}
