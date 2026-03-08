import { Box, Dialog, DialogContent, DialogTitle } from '@mui/material'
import type { IAddAdminUserPermissionProps } from '../scripts/IAdmin'
import AdminUserPermissionTable from './AdminUserPermissionTable'

export default function AdminAddUserPermissionDialog({
	selectedItems,
	openAddDialog,
	handleCloseAdd,
}: IAddAdminUserPermissionProps) {
	return (
		<Dialog
			sx={{ width: '100%' }}
			open={openAddDialog}
			onClose={() => handleCloseAdd()}
			closeAfterTransition={false}
			fullWidth
			maxWidth='xl'>
			<DialogTitle>User permission</DialogTitle>
			<DialogContent>
				<Box sx={{ display: 'flex', flexDirection: 'column', justifyContent: 'center', alignItems: 'center' }}>
					{selectedItems?.[0]?.id && <AdminUserPermissionTable userId={selectedItems[0].id} isAdmin={true} />}
				</Box>
			</DialogContent>
		</Dialog>
	)
}
