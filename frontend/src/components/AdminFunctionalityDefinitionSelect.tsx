import { useEffect } from 'react'
import type { FunctionalityDefinitionClass } from '../modules/Admin/scripts/FunctionalityDefinitionClass'
import { TextField, Autocomplete, useMediaQuery, useTheme } from '@mui/material'
import { useGetFunctionalityDefinitionsQuery } from '../store/api/adminApi'
import { useAppDispatch } from '../store/hooks'
import { showAlert } from '../store/application-store'

interface ISelectProps {
	getItem: (item: FunctionalityDefinitionClass | null) => void
	item: FunctionalityDefinitionClass | null | undefined
}

export default function AdminFunctionalityDefinitionSelect({ getItem, item }: ISelectProps) {
	const dispatch = useAppDispatch()

	const { data: functionalityDefinition = [], error: functionalityDefinitionError } =
		useGetFunctionalityDefinitionsQuery({})

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (functionalityDefinitionError) {
			const message =
				(functionalityDefinitionError as any)?.data?.message ||
				(functionalityDefinitionError as any)?.message ||
				'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [dispatch, functionalityDefinitionError])

	function getOptionLabel(functionality: FunctionalityDefinitionClass): string {
		return functionality.name || ''
	}

	return (
		<Autocomplete
			sx={{ mt: '1rem', width: isMobile ? 200 : 400 }}
			onChange={(_, value) => getItem(value)}
			disablePortal
			value={item}
			getOptionLabel={getOptionLabel}
			options={functionalityDefinition}
			slotProps={{ listbox: { sx: { maxHeight: '100px' } } }}
			renderInput={params => <TextField {...params} label='Functionality definition' />}
		/>
	)
}
