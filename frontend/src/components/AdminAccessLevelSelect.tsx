import { useEffect } from 'react'
import type { AccessLevelDefinitionClass } from '../modules/Admin/scripts/AccessLevelDefinitionClass'
import { TextField, Autocomplete, useMediaQuery, useTheme } from '@mui/material'
import { useGetAccessLevelDefinitionsQuery } from '../store/api/adminApi'
import { useAppDispatch } from '../store/hooks'
import { showAlert } from '../store/application-store'

interface ISelectProps {
	getItem: (item: AccessLevelDefinitionClass | null) => void
	item: AccessLevelDefinitionClass | null | undefined
}

export default function AdminAccessLevelDefinitionSelect({ getItem, item }: ISelectProps) {
	const dispatch = useAppDispatch()

	const { data: accessLevelDefinition = [], error: accessLevelDefinitionError } = useGetAccessLevelDefinitionsQuery({})

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (accessLevelDefinitionError) {
			const message =
				(accessLevelDefinitionError as any)?.data?.message ||
				(accessLevelDefinitionError as any)?.message ||
				'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [dispatch, accessLevelDefinitionError])

	function getOptionLabel(access: AccessLevelDefinitionClass): string {
		return access.name || ''
	}

	return (
		<Autocomplete
			sx={{ mt: '1rem', width: isMobile ? 200 : 400 }}
			onChange={(_, value) => getItem(value)}
			disablePortal
			value={item}
			getOptionLabel={getOptionLabel}
			options={accessLevelDefinition}
			slotProps={{ listbox: { sx: { maxHeight: '100px' } } }}
			renderInput={params => <TextField {...params} label='Access level' />}
		/>
	)
}
