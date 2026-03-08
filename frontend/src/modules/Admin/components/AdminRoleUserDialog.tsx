import { Box, Dialog, DialogContent, DialogTitle } from '@mui/material'
import type { IAddAdminRoleUserProps } from '../scripts/IAdmin'
import AdminUserRoleMain from './AdminUserRoleMain'

export default function AdminAddRoleUserDialog({
	selectedItems,
	openAddDialog,
	handleCloseAdd,
}: IAddAdminRoleUserProps) {
	return (
		<Dialog
			sx={{ width: '100%' }}
			open={openAddDialog}
			onClose={() => handleCloseAdd()}
			closeAfterTransition={false}
			fullWidth
			maxWidth='xl'>
			<DialogTitle>Role users</DialogTitle>
			<DialogContent>
				<Box sx={{ display: 'flex', flexDirection: 'column', justifyContent: 'center', alignItems: 'center' }}>
					{selectedItems?.[0]?.id && <AdminUserRoleMain roleId={selectedItems?.[0]?.id} isAdmin={true} />}
				</Box>
			</DialogContent>
		</Dialog>
	)
}
