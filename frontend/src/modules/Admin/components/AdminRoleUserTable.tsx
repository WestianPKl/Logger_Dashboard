import { useState, useEffect, useMemo, useCallback } from 'react'
import {
	Box,
	Typography,
	Icon,
	Button,
	Dialog,
	DialogActions,
	DialogContent,
	DialogTitle,
	DialogContentText,
	useMediaQuery,
	useTheme,
	IconButton,
} from '@mui/material'
import AddIcon from '@mui/icons-material/Add'
import DeleteIcon from '@mui/icons-material/Delete'
import { showAlert } from '../../../store/application-store'
import { useAppDispatch } from '../../../store/hooks'
import {
	DataGrid,
	type GridColDef,
	type GridFilterModel,
	type GridRowSelectionModel,
	type GridSortModel,
} from '@mui/x-data-grid'
import type { UserClass } from '../../User/scripts/UserClass'
import { useAddAdminRoleUserMutation, useDeleteAdminRoleUserMutation } from '../../../store/api/adminApi'
import type { IAddAdminRoleUserDataDialog } from '../scripts/IAdmin'
import GroupIcon from '@mui/icons-material/Group'
import type { IRoleUserPermissionProps } from '../scripts/IAdmin'
import AddUserRoleDialog from './AddAdminRoleUserDialog'
import { useRevalidator } from 'react-router'

export default function AdminRoleUserTable({
	usersData,
	roleId,
	isAdmin,
	initSort,
	initFilter,
}: IRoleUserPermissionProps) {
	const dispatch = useAppDispatch()
	const revalidator = useRevalidator()

	const [selectedItems, setSelectedItems] = useState<UserClass[]>([])
	const [rowSelectionModel, setRowSelectionModel] = useState<GridRowSelectionModel>({
		type: 'include',
		ids: new Set(),
	})

	const [openDeleteDialog, setOpenDeleteDialog] = useState<boolean>(false)
	const [openAddDialog, setOpenAddDialog] = useState<boolean>(false)
	const [sortModel, setSortModel] = useState<GridSortModel>(initSort)
	const [filterModel, setFilterModel] = useState<GridFilterModel>(initFilter)

	const [addAdminRoleUser] = useAddAdminRoleUserMutation()
	const [deleteAdminRoleUser] = useDeleteAdminRoleUserMutation()

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	const usersDataMap = useMemo(() => {
		const map = new Map()
		usersData.forEach(item => {
			if (item.id) map.set(item.id, item)
		})
		return map
	}, [usersData])

	useEffect(() => {
		localStorage.setItem('adminRoleUserTableSortModel', JSON.stringify(sortModel))
	}, [sortModel])

	useEffect(() => {
		localStorage.setItem('adminRoleUserTableFilterModel', JSON.stringify(filterModel))
	}, [filterModel])

	useEffect(() => {
		const selectedIds = [...rowSelectionModel.ids]
		setSelectedItems(selectedIds.map(id => usersDataMap.get(Number(id))).filter(Boolean))
	}, [rowSelectionModel, usersDataMap])

	async function addItemHandler(item: IAddAdminRoleUserDataDialog | IAddAdminRoleUserDataDialog[]): Promise<void> {
		try {
			setOpenAddDialog(false)
			if (!Array.isArray(item)) {
				if (item.user && item.user.length > 0) {
					await Promise.all(
						item.user.map(async user => {
							await addAdminRoleUser({ roleId: item.roleId, userId: user.id }).unwrap()
						}),
					)
				}
			}
			dispatch(showAlert({ message: 'User added', severity: 'success' }))
			revalidator.revalidate()
		} catch (err: any) {
			const message = err?.data?.message || err?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}

	async function deleteItemHandler(): Promise<void> {
		try {
			setOpenDeleteDialog(false)
			if (selectedItems.length >= 1) {
				await Promise.all(
					selectedItems.map(async item => {
						await deleteAdminRoleUser({ userId: item.id, roleId: roleId }).unwrap()
					}),
				)
				dispatch(showAlert({ message: 'Equipment vendor deleted', severity: 'success' }))
				revalidator.revalidate()
			}
		} catch (err: any) {
			const message = err?.data?.message || err?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}

	function handleClickAddOpen(): void {
		setOpenAddDialog(true)
	}

	function handleClickDeleteOpen(): void {
		setOpenDeleteDialog(true)
	}

	function handleCloseDelete(): void {
		setOpenDeleteDialog(false)
	}

	function handleCloseAdd(): void {
		setOpenAddDialog(false)
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
						<GroupIcon />
					</Icon>
					<Typography variant='h6' component='p'>
						Role Users
					</Typography>
				</Box>
				<Typography component='span'>Database containg all the role users.</Typography>
				<Box sx={{ mt: '2rem' }}>
					<Box sx={{ mb: '1rem', textAlign: 'right' }}>
						{isAdmin && roleId && (
							<>
								{!isMobile ? (
									<Button variant='contained' type='button' size='medium' onClick={handleClickAddOpen}>
										Add new user
									</Button>
								) : (
									<IconButton type='button' size='small' color='primary' onClick={handleClickAddOpen}>
										<AddIcon />
									</IconButton>
								)}
								<AddUserRoleDialog
									roleId={roleId}
									handleCloseAdd={handleCloseAdd}
									openAddDialog={openAddDialog}
									addItemHandler={addItemHandler}
								/>

								{selectedItems.length > 0 && (
									<>
										{!isMobile ? (
											<Button
												sx={{ ml: '0.3rem' }}
												variant='contained'
												color='error'
												type='button'
												size={isMobile ? 'small' : 'medium'}
												onClick={handleClickDeleteOpen}>
												Delete
											</Button>
										) : (
											<IconButton
												sx={{ ml: '0.3rem' }}
												color='error'
												type='button'
												size={isMobile ? 'small' : 'medium'}
												onClick={handleClickDeleteOpen}>
												<DeleteIcon />
											</IconButton>
										)}
									</>
								)}
							</>
						)}
						<Dialog open={openDeleteDialog} onClose={handleCloseDelete} closeAfterTransition={false}>
							<DialogTitle>Do you want to delete selected item(s)?</DialogTitle>
							<DialogContent>
								<DialogContentText>You have selected {selectedItems.length} item(s) to delete.</DialogContentText>
							</DialogContent>
							<DialogActions>
								<Button variant='outlined' size={isMobile ? 'small' : 'medium'} onClick={handleCloseDelete}>
									Cancel
								</Button>
								<Button
									variant='outlined'
									size={isMobile ? 'small' : 'medium'}
									onClick={deleteItemHandler}
									autoFocus
									color='error'>
									Delete
								</Button>
							</DialogActions>
						</Dialog>
					</Box>
				</Box>
			</Box>
			<Box sx={{ mt: '2rem' }}>
				<DataGrid
					rows={usersData}
					columns={useMemo<GridColDef[]>(
						() => [
							{ field: 'id', headerName: 'ID', width: 50 },
							{
								field: 'username',
								headerName: 'Username',
								width: 300,
								valueGetter: (_, row) => `${row.username ? row.username : '-'}`,
							},
							{
								field: 'email',
								headerName: 'Email',
								width: 300,
								valueGetter: (_, row) => `${row.email ? row.email : '-'}`,
							},
						],
						[],
					)}
					initialState={{ pagination: { paginationModel: { page: 0, pageSize: 15 } } }}
					pageSizeOptions={[15, 30, 45]}
					disableRowSelectionOnClick={true}
					checkboxSelection={isAdmin && roleId ? true : false}
					sx={{
						border: 'none',
						borderRadius: 3,
						'& .MuiDataGrid-columnHeaders': { backgroundColor: 'rgba(74, 158, 158, 0.06)', fontWeight: 600 },
						'& .MuiDataGrid-row:hover': { backgroundColor: 'rgba(74, 158, 158, 0.04)' },
						'& .MuiDataGrid-cell': { borderColor: 'rgba(0, 0, 0, 0.04)' },
					}}
					density='compact'
					disableColumnResize={true}
					disableColumnSelector={true}
					disableVirtualization={true}
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
