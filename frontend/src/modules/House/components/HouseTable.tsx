import { useState, useEffect, useMemo, useCallback } from 'react'
import {
	Box,
	Typography,
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
import EditIcon from '@mui/icons-material/Edit'
import DeleteIcon from '@mui/icons-material/Delete'
import {
	DataGrid,
	type GridColDef,
	type GridFilterModel,
	type GridRowSelectionModel,
	type GridSortModel,
} from '@mui/x-data-grid'
import type { IAddHouseData, IHouseTableProps } from '../scripts/IHouse'
import HouseIcon from '@mui/icons-material/House'
import type { HouseClass } from '../scripts/HouseClass'
import AddHouseDialog from './AddHouseDialog'
import { showAlert } from '../../../store/application-store'
import { canWrite, canDelete } from '../../../store/auth-actions'
import { useAppDispatch, useAppSelector } from '../../../store/hooks'
import { useAddHouseMutation, useUpdateHouseMutation, useDeleteHouseMutation } from '../../../store/api/houseApi'
import { useRevalidator } from 'react-router'
import formatLocalDateTime from '../../../components/scripts/ComponentsInterface'

export default function HouseTable({ houses, initSort, initFilter }: IHouseTableProps) {
	const dispatch = useAppDispatch()
	const revalidator = useRevalidator()

	const [selectedItems, setSelectedItems] = useState<HouseClass[]>([])
	const [rowSelectionModel, setRowSelectionModel] = useState<GridRowSelectionModel>({
		type: 'include',
		ids: new Set(),
	})
	const [openDeleteDialog, setOpenDeleteDialog] = useState<boolean>(false)
	const [openAddDialog, setOpenAddDialog] = useState<boolean>(false)
	const [openEditDialog, setOpenEditDialog] = useState<boolean>(false)
	const [sortModel, setSortModel] = useState<GridSortModel>(initSort)
	const [filterModel, setFilterModel] = useState<GridFilterModel>(initFilter)

	const isWritable = useAppSelector(state => canWrite('house', 'houseHouse')(state))
	const isDeletable = useAppSelector(state => canDelete('house', 'houseHouse')(state))

	const [addHouse] = useAddHouseMutation()
	const [updateHouse] = useUpdateHouseMutation()
	const [deleteHouse] = useDeleteHouseMutation()

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	const columns = useMemo<GridColDef[]>(
		() => [
			{ field: 'id', headerName: 'ID', width: 50 },
			{ field: 'name', headerName: 'Name', width: 150 },
			{ field: 'postalCode', headerName: 'Postal code', width: 150 },
			{ field: 'city', headerName: 'City', width: 150 },
			{ field: 'street', headerName: 'Street', width: 150 },
			{ field: 'houseNumber', headerName: 'House number', width: 150 },
			{
				field: 'createdBy.username',
				headerName: 'Created by',
				width: 155,
				valueGetter: (_, row) => `${row.createdBy.username}`,
			},
			{
				field: 'updatedBy.username',
				headerName: 'Updated by',
				width: 155,
				valueGetter: (_, row) => `${row.updatedBy.username}`,
			},
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
		],
		[],
	)

	const housesMap = useMemo(() => {
		const map = new Map()
		houses.forEach(item => {
			if (item.id) map.set(item.id, item)
		})
		return map
	}, [houses])

	useEffect(() => {
		localStorage.setItem('houseTableSortModel', JSON.stringify(sortModel))
	}, [sortModel])

	useEffect(() => {
		localStorage.setItem('houseTableFilterModel', JSON.stringify(filterModel))
	}, [filterModel])

	useEffect(() => {
		const selectedIds = [...rowSelectionModel.ids]
		setSelectedItems(selectedIds.map(id => housesMap.get(Number(id))).filter(Boolean))
	}, [rowSelectionModel, housesMap])

	function clearObject(): void {
		setSelectedItems([])
		setRowSelectionModel({
			type: 'include',
			ids: new Set(),
		})
	}

	async function addItemHandler(item: IAddHouseData | IAddHouseData[]): Promise<void> {
		try {
			setOpenAddDialog(false)
			if (!Array.isArray(item)) {
				const formData = new FormData()
				if (item.name) {
					formData.append('name', item.name)
				}
				if (item.postalCode) {
					formData.append('postalCode', item.postalCode)
				}
				if (item.city) {
					formData.append('city', item.city)
				}
				if (item.street) {
					formData.append('street', item.street)
				}
				if (item.houseNumber) {
					formData.append('houseNumber', item.houseNumber)
				}
				if (item.pictureLink) {
					formData.append('pictureLink', item.pictureLink)
				}
				await addHouse(formData).unwrap()
			}
			dispatch(showAlert({ message: 'New house added', severity: 'success' }))
			revalidator.revalidate()
		} catch (err: any) {
			const message = err?.data?.message || err?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}

	async function editItemHandler(items: IAddHouseData | IAddHouseData[]): Promise<void> {
		try {
			setOpenEditDialog(false)
			if (Array.isArray(items) && items.length >= 1) {
				await Promise.all(
					items.map(async item => {
						const formData = new FormData()
						if (item.name) {
							formData.append('name', item.name)
						}
						if (item.postalCode) {
							formData.append('postalCode', item.postalCode)
						}
						if (item.city) {
							formData.append('city', item.city)
						}
						if (item.street) {
							formData.append('street', item.street)
						}
						if (item.houseNumber) {
							formData.append('houseNumber', item.houseNumber)
						}
						if (item.pictureLink) {
							formData.append('pictureLink', item.pictureLink)
						}
						if (item.id) {
							await updateHouse({ body: formData, id: item.id })
						}
					}),
				)
				dispatch(showAlert({ message: 'House edited', severity: 'success' }))
				clearObject()
				revalidator.revalidate()
			}
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
						await deleteHouse({ id: item.id })
					}),
				)
				dispatch(showAlert({ message: 'House deleted', severity: 'success' }))
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

	function handleClickEditOpen(): void {
		setOpenEditDialog(true)
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

	function handleCloseEdit(): void {
		setOpenEditDialog(false)
	}

	const handleSortModelChange = useCallback((newSortModel: GridSortModel) => {
		setSortModel(newSortModel)
	}, [])

	const handleRowSelectionModelChange = useCallback((newRowSelectionModel: GridRowSelectionModel) => {
		setRowSelectionModel(newRowSelectionModel)
	}, [])

	return (
		<Box sx={{ textAlign: 'center' }}>
			<Box sx={{ textAlign: 'left', mb: 3 }}>
				<Box sx={{ display: 'flex', alignItems: 'center', gap: 1, mb: 0.5 }}>
					<HouseIcon sx={{ color: 'primary.main', fontSize: 28 }} />
					<Typography variant='h6' sx={{ fontWeight: 700 }}>
						Houses database
					</Typography>
				</Box>
				<Typography variant='body2' sx={{ color: 'text.secondary' }}>
					Your database containing all the houses you have registered.
				</Typography>
			</Box>
			<Box sx={{ mt: '2rem' }}>
				<Box sx={{ mb: '1rem', textAlign: 'right' }}>
					{isWritable && (
						<>
							{!isMobile ? (
								<Button variant='contained' type='button' size='medium' onClick={handleClickAddOpen}>
									Add new house
								</Button>
							) : (
								<IconButton type='button' size='small' color='primary' onClick={handleClickAddOpen}>
									<AddIcon />
								</IconButton>
							)}
							<AddHouseDialog
								edit={false}
								handleCloseAdd={handleCloseAdd}
								openAddDialog={openAddDialog}
								addItemHandler={addItemHandler}
							/>
						</>
					)}
					{selectedItems.length > 0 && (
						<>
							{isWritable && (
								<>
									{!isMobile ? (
										<Button
											sx={{ ml: '0.3rem' }}
											variant='contained'
											color='info'
											type='button'
											size={isMobile ? 'small' : 'medium'}
											onClick={handleClickEditOpen}>
											Edit
										</Button>
									) : (
										<IconButton
											sx={{ ml: '0.3rem' }}
											color='info'
											type='button'
											size={isMobile ? 'small' : 'medium'}
											onClick={handleClickEditOpen}>
											<EditIcon />
										</IconButton>
									)}
									<AddHouseDialog
										edit={true}
										handleCloseAdd={handleCloseEdit}
										openAddDialog={openEditDialog}
										selectedItems={selectedItems}
										addItemHandler={editItemHandler}
									/>
								</>
							)}
							{isDeletable && (
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
						</>
					)}
				</Box>

				<DataGrid
					rows={houses}
					columns={columns}
					initialState={{ pagination: { paginationModel: { page: 0, pageSize: 15 } } }}
					pageSizeOptions={[15, 30, 45]}
					checkboxSelection={isWritable ? true : false}
					disableRowSelectionOnClick={true}
					sx={{
						border: 'none',
						borderRadius: 3,
						'& .MuiDataGrid-columnHeaders': {
							backgroundColor: 'rgba(74, 158, 158, 0.06)',
							fontWeight: 600,
						},
						'& .MuiDataGrid-row:hover': {
							backgroundColor: 'rgba(74, 158, 158, 0.04)',
						},
						'& .MuiDataGrid-cell': {
							borderColor: 'rgba(0, 0, 0, 0.04)',
						},
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
