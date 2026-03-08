import { useState } from 'react'
import { Box, Button, Dialog, DialogActions, DialogContent, DialogTitle, useMediaQuery, useTheme } from '@mui/material'
import type { IAddAdminUserPermissionDialogProps } from '../scripts/IAdmin'
import AdminFunctionalityDefinitionSelect from '../../../components/AdminFunctionalityDefinitionSelect'
import AdminObjectDefinitionSelect from '../../../components/AdminObjectDefinitionSelect'
import AdminAccessLevelDefinitionSelect from '../../../components/AdminAccessLevelSelect'
import type { FunctionalityDefinitionClass } from '../scripts/FunctionalityDefinitionClass'
import type { ObjectDefinitionClass } from '../scripts/ObjectDefinitionClass'
import type { AccessLevelDefinitionClass } from '../scripts/AccessLevelDefinitionClass'

export default function AddUserPermissionDialog({
	userId,
	roleId,
	openAddDialog,
	handleCloseAdd,
	addItemHandler,
}: IAddAdminUserPermissionDialogProps) {
	const [functionalityDefinition, setFunctionalityDefinition] = useState<FunctionalityDefinitionClass | null>(null)
	const [objectDefinition, setObjectDefinition] = useState<ObjectDefinitionClass | null>(null)
	const [accessLevelDefinition, setAccessLevelDefinition] = useState<AccessLevelDefinitionClass | null>(null)

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	function onSubmitHandler(e: React.FormEvent): void {
		e.preventDefault()
		if (!functionalityDefinition || !accessLevelDefinition) return
		const data = {
			userId,
			roleId,
			admFunctionalityDefinitionId: functionalityDefinition.id,
			admObjectDefinitionId: objectDefinition?.id ?? null,
			admAccessLevelDefinitionId: accessLevelDefinition.id,
		}
		closeDialog()
		addItemHandler(data)
	}

	function closeDialog(): void {
		setFunctionalityDefinition(null)
		setObjectDefinition(null)
		setAccessLevelDefinition(null)
		handleCloseAdd()
	}

	return (
		<Dialog sx={{ width: '100%' }} open={openAddDialog} onClose={closeDialog} closeAfterTransition={false}>
			<DialogTitle>Add permission</DialogTitle>
			<Box sx={{ padding: 0, margin: 0 }} onSubmit={onSubmitHandler} component='form' noValidate autoComplete='off'>
				<DialogContent>
					<Box sx={{ display: 'flex', flexDirection: 'column', justifyContent: 'center', alignItems: 'center' }}>
						<AdminFunctionalityDefinitionSelect getItem={setFunctionalityDefinition} item={functionalityDefinition} />
						<AdminObjectDefinitionSelect getItem={setObjectDefinition} item={objectDefinition} />
						<AdminAccessLevelDefinitionSelect getItem={setAccessLevelDefinition} item={accessLevelDefinition} />
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
						disabled={!functionalityDefinition || !accessLevelDefinition}>
						Add
					</Button>
				</DialogActions>
			</Box>
		</Dialog>
	)
}
