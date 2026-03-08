import { Box, Dialog, DialogContent, DialogTitle } from '@mui/material'
import type { IAddAdminRolePermissionProps } from '../scripts/IAdmin'
import AdminUserPermissionTable from './AdminUserPermissionTable'

export default function AdminAddRolePermissionDialog({
	selectedItems,
	openAddDialog,
	handleCloseAdd,
}: IAddAdminRolePermissionProps) {
	return (
		<Dialog
			sx={{ width: '100%' }}
			open={openAddDialog}
			onClose={() => handleCloseAdd()}
			closeAfterTransition={false}
			fullWidth
			maxWidth='xl'>
			<DialogTitle>Role permission</DialogTitle>
			<DialogContent>
				<Box sx={{ display: 'flex', flexDirection: 'column', justifyContent: 'center', alignItems: 'center' }}>
					{selectedItems?.[0]?.id && <AdminUserPermissionTable roleId={selectedItems[0].id} isAdmin={true} />}
				</Box>
			</DialogContent>
		</Dialog>
	)
}
