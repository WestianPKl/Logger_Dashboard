import { useEffect, useState } from 'react'
import {
	Box,
	TextField,
	Button,
	Dialog,
	DialogActions,
	DialogContent,
	DialogTitle,
	useMediaQuery,
	useTheme,
} from '@mui/material'
import type { IAddDataDefinitionProps } from '../scripts/IData'

export default function AddDataDefinitionDialog({
	edit,
	selectedItems,
	openAddDialog,
	handleCloseAdd,
	addItemHandler,
}: IAddDataDefinitionProps) {
	const [name, setName] = useState('')
	const [unit, setUnit] = useState('')
	const [description, setDescription] = useState('')
	const [multiple, setMultiple] = useState(false)
	const [itemId, setItemId] = useState<number | undefined>(undefined)

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (edit) {
			if (selectedItems?.length === 1) {
				setName(selectedItems[0].name || '')
				setUnit(selectedItems[0].unit || '')
				setDescription(selectedItems[0].description || '')
				setItemId(selectedItems[0].id)
				setMultiple(false)
			} else {
				setName('')
				setUnit('')
				setDescription('')
				setItemId(undefined)
				setMultiple(true)
			}
		} else {
			setName('')
			setUnit('')
			setDescription('')
			setItemId(undefined)
			setMultiple(false)
		}
	}, [openAddDialog, edit, selectedItems])

	function onNameChangeHandler(e: React.ChangeEvent<HTMLInputElement>): void {
		setName(e.target.value)
	}

	function closeDialog(): void {
		handleCloseAdd()
	}

	function onSubmitHandler(e: React.FormEvent): void {
		e.preventDefault()
		if (!edit) {
			addItemHandler({ name, unit, description })
		} else if (edit && multiple) {
			addItemHandler(
				selectedItems?.map(e => ({
					id: e.id,
					name: e.name,
					unit: e.unit,
					description: e.description,
				})) || [],
			)
		} else if (edit && !multiple) {
			addItemHandler([{ id: itemId, name, unit, description }])
		}
		closeDialog()
	}

	return (
		<Dialog sx={{ width: '100%' }} open={openAddDialog} onClose={closeDialog} closeAfterTransition={false}>
			<DialogTitle>{edit ? 'Edit data definition' : 'Add data definition'}</DialogTitle>
			<Box sx={{ padding: 0, margin: 0 }} onSubmit={onSubmitHandler} component='form' noValidate autoComplete='off'>
				<DialogContent>
					<Box sx={{ display: 'flex', flexDirection: 'column', justifyContent: 'center', alignItems: 'center' }}>
						<TextField
							sx={{ mt: 1, width: isMobile ? 200 : 400 }}
							id='name'
							label='Name'
							onChange={onNameChangeHandler}
							disabled={multiple}
							value={name}
						/>
						<TextField
							sx={{ mt: 1, width: isMobile ? 200 : 400 }}
							id='unit'
							label='Unit'
							onChange={e => setUnit(e.target.value)}
							disabled={multiple}
							value={unit}
						/>
						<TextField
							sx={{ mt: 1, width: isMobile ? 200 : 400 }}
							id='description'
							label='Description'
							onChange={e => setDescription(e.target.value)}
							disabled={multiple}
							value={description}
						/>
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
						disabled={!name.trim() || (edit && multiple)}>
						{edit ? 'Save' : 'Add'}
					</Button>
				</DialogActions>
			</Box>
		</Dialog>
	)
}
