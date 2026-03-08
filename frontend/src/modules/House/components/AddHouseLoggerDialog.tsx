import { useEffect, useState } from 'react'
import { Box, Button, Dialog, DialogActions, DialogContent, DialogTitle, useMediaQuery, useTheme } from '@mui/material'
import type { IAddHouseLoggerProps } from '../scripts/IHouse'
import type { EquipmentClass } from '../../Equipment/scripts/EquipmentClass'
import EquipmentLoggerSelect from '../../../components/EquipmentLoggerSelect'
import HouseFloorSelect from '../../../components/HouseFloorSelect'
import type { HouseFloorClass } from '../scripts/HouseFloorClass'

export default function AddHouseLoggerDialog({
	edit,
	selectedItems,
	openAddDialog,
	handleCloseAdd,
	addItemHandler,
}: IAddHouseLoggerProps) {
	const [logger, setLogger] = useState<EquipmentClass | null>(null)
	const [houseFloor, setHouseFloor] = useState<HouseFloorClass | null>(null)
	const [multiple, setMultiple] = useState(false)
	const [itemId, setItemId] = useState<number | undefined>(undefined)

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (edit) {
			if (selectedItems?.length === 1) {
				const item = selectedItems[0]
				setItemId(item.id)
				setLogger(item.logger || null)
				setHouseFloor(item.floor || null)
				setMultiple(false)
			} else {
				setItemId(undefined)
				setLogger(null)
				setHouseFloor(null)
				setMultiple(true)
			}
		} else {
			setItemId(undefined)
			setLogger(null)
			setHouseFloor(null)
			setMultiple(false)
		}
	}, [openAddDialog, edit, selectedItems])

	function onLoggerChangeHandler(item: EquipmentClass | null): void {
		setLogger(item)
	}

	function onHouseFloorChangeHandler(item: HouseFloorClass | null): void {
		setHouseFloor(item)
	}

	function closeDialog(): void {
		handleCloseAdd()
	}

	function onSubmitHandler(e: React.FormEvent): void {
		e.preventDefault()
		if (!edit) {
			addItemHandler({
				equLoggerId: logger?.id,
				houseFloorId: houseFloor?.id,
			})
		} else if (edit && multiple) {
			addItemHandler(
				selectedItems?.map(e => ({
					id: e.id,
					equLoggerId: e.equLoggerId,
					houseFloorId: e.houseFloorId,
				})) || [],
			)
		} else if (edit && !multiple) {
			addItemHandler([
				{
					id: itemId,
					equLoggerId: logger?.id,
					houseFloorId: houseFloor?.id,
				},
			])
		}
		closeDialog()
	}

	return (
		<Dialog sx={{ width: '100%' }} open={openAddDialog} onClose={closeDialog} closeAfterTransition={false}>
			<DialogTitle>{edit ? 'Edit house Logger' : 'Add house Logger'}</DialogTitle>
			<Box sx={{ padding: 0, margin: 0 }} onSubmit={onSubmitHandler} component='form' noValidate autoComplete='off'>
				<DialogContent>
					<Box
						sx={{
							width: isMobile ? 200 : 400,
							height: 150,
							display: 'flex',
							flexDirection: 'column',
							justifyContent: 'center',
							alignItems: 'center',
						}}>
						<EquipmentLoggerSelect getItem={onLoggerChangeHandler} item={logger} />
						<HouseFloorSelect getItem={onHouseFloorChangeHandler} item={houseFloor} />
					</Box>
				</DialogContent>
				<DialogActions>
					<Button variant='outlined' size={isMobile ? 'small' : 'medium'} onClick={closeDialog}>
						Cancel
					</Button>
					<Button
						variant='outlined'
						size={isMobile ? 'small' : 'medium'}
						type='submit'
						disabled={!logger || !houseFloor || (edit && multiple)}>
						{edit ? 'Save' : 'Add'}
					</Button>
				</DialogActions>
			</Box>
		</Dialog>
	)
}
