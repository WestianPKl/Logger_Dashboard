import {
	Box,
	Button,
	Dialog,
	useMediaQuery,
	useTheme,
	DialogActions,
	DialogContent,
	DialogTitle,
	IconButton,
} from '@mui/material'
import EquipmentLoggerSelect from '../../../components/EquipmentLoggerSelect'
import DeleteIcon from '@mui/icons-material/Delete'
import type { IHouseDetailsNewLoggerNodeDialogProps } from '../scripts/IHouseDetails'
import { useState } from 'react'
import { EquipmentClass } from '../../Equipment/scripts/EquipmentClass'

export default function HouseDetailsLoggerNewNodeDialog({
	loggerData,
	detailsDialog,
	onCloseDialog,
	addItemHandler,
	handleClickDeleteNode,
}: IHouseDetailsNewLoggerNodeDialogProps) {
	const [logger, setLogger] = useState<EquipmentClass | null | undefined>(null)

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	function onLoggerChangeHandler(item: EquipmentClass | null | undefined): void {
		setLogger(item)
	}

	function onSubmitHandler(e: React.FormEvent): void {
		e.preventDefault()
		closeDialog()
		addItemHandler({
			equLoggerId: logger?.id,
			houseFloorId: loggerData.floorId,
		})
	}

	function closeDialog(): void {
		setLogger(null)
		onCloseDialog()
	}

	return (
		<Dialog fullWidth maxWidth='sm' open={detailsDialog} onClose={closeDialog} closeAfterTransition={false}>
			<DialogTitle sx={{ textAlign: 'center', position: 'relative', px: 2 }}>
				New logger
				<IconButton
					sx={{ position: 'absolute', right: 8, top: 8 }}
					color='error'
					type='button'
					size={isMobile ? 'small' : 'medium'}
					onClick={() => handleClickDeleteNode(loggerData)}
					aria-label='delete'>
					<DeleteIcon />
				</IconButton>
			</DialogTitle>
			<Box sx={{ padding: 0, margin: 0 }} onSubmit={onSubmitHandler} component='form' noValidate autoComplete='off'>
				<DialogContent>
					<Box
						sx={{
							width: isMobile ? 200 : 400,
							height: 140,
							display: 'flex',
							flexDirection: 'column',
							justifyContent: 'center',
							alignItems: 'center',
						}}>
						<EquipmentLoggerSelect getItem={onLoggerChangeHandler} item={logger} />
					</Box>
				</DialogContent>
				<DialogActions>
					<Button variant='outlined' size={isMobile ? 'small' : 'medium'} onClick={closeDialog}>
						Cancel
					</Button>
					<Button variant='outlined' size={isMobile ? 'small' : 'medium'} type='submit' disabled={!logger}>
						Save
					</Button>
				</DialogActions>
			</Box>
		</Dialog>
	)
}
