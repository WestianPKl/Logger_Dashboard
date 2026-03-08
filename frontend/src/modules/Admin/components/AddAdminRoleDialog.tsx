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
import type { IAddAdminRoleProps } from '../scripts/IAdmin'

export default function AdminAddAdminRoleDialog({
	edit,
	selectedItems,
	openAddDialog,
	handleCloseAdd,
	addItemHandler,
}: IAddAdminRoleProps) {
	const [name, setName] = useState<string>('')
	const [description, setDescription] = useState<string | undefined>(undefined)
	const [multiple, setMultiple] = useState<boolean>(false)
	const [itemId, setItemId] = useState<number | undefined>(undefined)

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (edit) {
			if (selectedItems?.length === 1) {
				setName(selectedItems[0].name || '')
				setDescription(selectedItems[0].description)
				setItemId(selectedItems[0].id)
				setMultiple(false)
			} else {
				setName('')
				setDescription(undefined)
				setItemId(undefined)
				setMultiple(true)
			}
		} else {
			setName('')
			setDescription(undefined)
			setItemId(undefined)
			setMultiple(false)
		}
	}, [openAddDialog, selectedItems, edit])

	function onNameChangeHandler(e: React.ChangeEvent<HTMLInputElement>): void {
		setName(e.target.value)
	}
	function onDescriptionChangeHandler(e: React.ChangeEvent<HTMLInputElement>): void {
		setDescription(e.target.value)
	}

	function onSubmitHandler(e: React.FormEvent): void {
		e.preventDefault()
		if (!edit) {
			closeDialog()
			addItemHandler({ name, description })
		} else if (edit && multiple) {
			closeDialog()
			addItemHandler(
				selectedItems?.map(e => ({
					id: e.id,
					name: e.name,
					description: e.description,
				})) || [],
			)
		} else if (edit && !multiple) {
			closeDialog()
			addItemHandler([{ id: itemId, name, description }])
		}
	}

	function closeDialog(): void {
		setName('')
		setDescription('')
		setItemId(undefined)
		handleCloseAdd()
	}

	return (
		<Dialog sx={{ width: '100%' }} open={openAddDialog} onClose={closeDialog} closeAfterTransition={false}>
			<DialogTitle>{edit ? 'Edit Role' : 'Add Role'}</DialogTitle>
			<Box sx={{ padding: 0, margin: 0 }} onSubmit={onSubmitHandler} component='form' noValidate autoComplete='off'>
				<DialogContent>
					<Box sx={{ display: 'flex', flexDirection: 'column', justifyContent: 'center', alignItems: 'center' }}>
						<TextField
							sx={{ mt: '1rem', width: isMobile ? 200 : 400 }}
							id='name'
							label='Name'
							onChange={onNameChangeHandler}
							disabled={multiple}
							value={name}
							autoFocus
						/>
						<TextField
							sx={{ mt: '1rem', width: isMobile ? 200 : 400 }}
							id='description'
							label='Description'
							onChange={onDescriptionChangeHandler}
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
						disabled={!name?.trim() || (edit && multiple)}>
						{edit ? 'Save' : 'Add'}
					</Button>
				</DialogActions>
			</Box>
		</Dialog>
	)
}
