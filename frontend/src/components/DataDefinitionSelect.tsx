import { useEffect } from 'react'
import type { DataDefinitionClass } from '../modules/Data/scripts/DataDefinitionClass'
import { TextField, Autocomplete, useMediaQuery, useTheme } from '@mui/material'
import { useGetDataDefinitionsQuery } from '../store/api/dataApi'
import { useAppDispatch } from '../store/hooks'
import { showAlert } from '../store/application-store'

interface ISelectProps {
	getItem: (item: DataDefinitionClass[]) => void
	item: DataDefinitionClass[] | undefined
}

export default function DataDefinitionSelect({ getItem, item }: ISelectProps) {
	const dispatch = useAppDispatch()

	const { data: dataDefinitions = [], error: dataDefinitionsError } = useGetDataDefinitionsQuery({})

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (dataDefinitionsError) {
			const message =
				(dataDefinitionsError as any)?.data?.message || (dataDefinitionsError as any)?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [dispatch, dataDefinitionsError])

	function getOptionLabel(dataDefinition: DataDefinitionClass): string {
		return dataDefinition.name || ''
	}

	return (
		<Autocomplete
			multiple
			limitTags={2}
			sx={{ mt: '1rem', width: isMobile ? 200 : 400 }}
			onChange={(_, value) => getItem(value)}
			disablePortal
			value={item ?? []}
			getOptionLabel={getOptionLabel}
			options={dataDefinitions}
			slotProps={{ listbox: { sx: { maxHeight: '100px' } } }}
			renderInput={params => <TextField {...params} label='Data definition' />}
		/>
	)
}
