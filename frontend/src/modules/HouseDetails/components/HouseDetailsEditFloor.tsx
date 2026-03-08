import { useState } from 'react'
import {
	Box,
	Button,
	Dialog,
	DialogActions,
	DialogTitle,
	DialogContent,
	DialogContentText,
	useMediaQuery,
	useTheme,
} from '@mui/material'
import type { IHouseDetailsFloorProps } from '../scripts/IHouseDetails'
import { useDeleteHouseFloorMutation, useUpdateHouseFloorMutation } from '../../../store/api/houseApi'
import { showAlert } from '../../../store/application-store'
import { useAppDispatch } from '../../../store/hooks'
import type { IAddHouseFloorData } from '../../House/scripts/IHouse'
import AddHouseFloorDialog from '../../House/components/AddHouseFloorDialog'
import { useRevalidator } from 'react-router'

export default function HouseDetailsEditFloor({ floor, houseId }: IHouseDetailsFloorProps) {
	const [openDeleteDialog, setOpenDeleteDialog] = useState<boolean>(false)
	const [openEditDialog, setOpenEditDialog] = useState<boolean>(false)
	const revalidator = useRevalidator()

	const dispatch = useAppDispatch()
	const [updateHouseFloor] = useUpdateHouseFloorMutation()
	const [deleteHouseFloor] = useDeleteHouseFloorMutation()

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	async function editItemHandler(items: IAddHouseFloorData | IAddHouseFloorData[]): Promise<void> {
		try {
			if (Array.isArray(items) && items.length >= 1) {
				await Promise.all(
					items.map(async item => {
						const formData = new FormData()
						if (item.name) {
							formData.append('name', item.name)
						}
						if (houseId) {
							formData.append('houseId', `${houseId}`)
						}
						if (item.layout) {
							formData.append('layout', item.layout)
						}
						if (item.id) {
							await updateHouseFloor({ body: formData, id: item.id }).unwrap()
						}
					}),
				)
				handleCloseEdit()
				dispatch(showAlert({ message: 'House floor edited', severity: 'success' }))
				revalidator.revalidate()
			}
		} catch (err: any) {
			const message = err?.data?.message || err?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}

	async function deleteItemHandler(): Promise<void> {
		try {
			if (floor.id) {
				await deleteHouseFloor({ id: floor.id }).unwrap()
				dispatch(showAlert({ message: 'House floor deleted', severity: 'success' }))
				revalidator.revalidate()
			}
		} catch (err: any) {
			const message = err?.data?.message || err?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
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

	function handleCloseEdit(): void {
		setOpenEditDialog(false)
	}

	return (
		<Box>
			<Button
				size={isMobile ? 'small' : 'medium'}
				sx={{ marginRight: '1rem' }}
				variant='contained'
				color='warning'
				onClick={handleClickEditOpen}>
				Edit floor
			</Button>
			{floor.loggers.length === 0 && (
				<Button
					size={isMobile ? 'small' : 'medium'}
					sx={{ marginRight: '1rem' }}
					variant='contained'
					color='error'
					onClick={handleClickDeleteOpen}>
					Delete floor
				</Button>
			)}
			<AddHouseFloorDialog
				isDashboard={true}
				edit={true}
				handleCloseAdd={handleCloseEdit}
				openAddDialog={openEditDialog}
				selectedItems={[floor]}
				addItemHandler={editItemHandler}
			/>
			<Dialog open={openDeleteDialog} onClose={handleCloseDelete} closeAfterTransition={false}>
				<DialogTitle>Do you want to delete selected item?</DialogTitle>
				<DialogContent>
					<DialogContentText>{`Selected floor ID${floor.id} ${floor.name} will be removed?`}</DialogContentText>
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
	)
}
