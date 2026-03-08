import { useState } from 'react'
import { Box, Button, Dialog, DialogActions, DialogContent, DialogTitle, useMediaQuery, useTheme } from '@mui/material'
import type { IAddAdminRoleUserDialogProps } from '../scripts/IAdmin'
import AdminUserSelect from '../../../components/AdminUserSelect'
import type { UserClass } from '../../User/scripts/UserClass'

export default function AddUserRoleDialog({
	roleId,
	openAddDialog,
	handleCloseAdd,
	addItemHandler,
}: IAddAdminRoleUserDialogProps) {
	const [users, setUsers] = useState<UserClass[] | undefined>(undefined)

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	function onSubmitHandler(e: React.FormEvent): void {
		e.preventDefault()
		if (!users || users.length === 0) return
		const data = {
			user: users,
			roleId,
		}
		closeDialog()
		addItemHandler(data)
	}

	function onUserChangeHandler(item: UserClass[]): void {
		setUsers(item)
	}

	function closeDialog(): void {
		setUsers(undefined)
		handleCloseAdd()
	}

	return (
		<Dialog sx={{ width: '100%' }} open={openAddDialog} onClose={closeDialog} closeAfterTransition={false}>
			<DialogTitle>Add user to role</DialogTitle>
			<Box sx={{ padding: 0, margin: 0 }} onSubmit={onSubmitHandler} component='form' noValidate autoComplete='off'>
				<DialogContent>
					<Box sx={{ display: 'flex', flexDirection: 'column', justifyContent: 'center', alignItems: 'center' }}>
						<AdminUserSelect getItem={onUserChangeHandler} item={users} />
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
						disabled={!users || users.length === 0}>
						Add
					</Button>
				</DialogActions>
			</Box>
		</Dialog>
	)
}
