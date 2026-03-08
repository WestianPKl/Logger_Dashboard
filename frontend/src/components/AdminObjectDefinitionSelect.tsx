import { useEffect } from 'react'
import type { ObjectDefinitionClass } from '../modules/Admin/scripts/ObjectDefinitionClass'
import { TextField, Autocomplete, useMediaQuery, useTheme } from '@mui/material'
import { useGetObjectDefinitionsQuery } from '../store/api/adminApi'
import { useAppDispatch } from '../store/hooks'
import { showAlert } from '../store/application-store'

interface ISelectProps {
	getItem: (item: ObjectDefinitionClass | null) => void
	item: ObjectDefinitionClass | null | undefined
}

export default function AdminObjectDefinitionSelect({ getItem, item }: ISelectProps) {
	const dispatch = useAppDispatch()

	const { data: objectDefinition = [], error: objectDefinitionError } = useGetObjectDefinitionsQuery({})

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (objectDefinitionError) {
			const message =
				(objectDefinitionError as any)?.data?.message ||
				(objectDefinitionError as any)?.message ||
				'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [dispatch, objectDefinitionError])

	function getOptionLabel(object: ObjectDefinitionClass): string {
		return object.name || ''
	}

	return (
		<Autocomplete
			sx={{ mt: '1rem', width: isMobile ? 200 : 400 }}
			onChange={(_, value) => getItem(value)}
			disablePortal
			value={item}
			getOptionLabel={getOptionLabel}
			options={objectDefinition}
			slotProps={{ listbox: { sx: { maxHeight: '100px' } } }}
			renderInput={params => <TextField {...params} label='Object definition' />}
		/>
	)
}
