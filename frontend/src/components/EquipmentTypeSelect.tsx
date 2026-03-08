import { useEffect } from 'react'
import type { EquipmentTypeClass } from '../modules/Equipment/scripts/EquipmentTypeClass'
import { TextField, Autocomplete, useMediaQuery, useTheme } from '@mui/material'
import { useGetEquipmentTypesQuery } from '../store/api/equipmentApi'
import { useAppDispatch } from '../store/hooks'
import { showAlert } from '../store/application-store'

interface ISelectProps {
	getItem: (item: EquipmentTypeClass | null) => void
	item: EquipmentTypeClass | null | undefined
}

export default function EquipmentTypeSelect({ getItem, item }: ISelectProps) {
	const dispatch = useAppDispatch()

	const { data: equipmentType = [], error: equipmentTypeError } = useGetEquipmentTypesQuery({})

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (equipmentTypeError) {
			const message =
				(equipmentTypeError as any)?.data?.message || (equipmentTypeError as any)?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [dispatch, equipmentTypeError])

	function getOptionLabel(type: EquipmentTypeClass): string {
		return type.name || ''
	}

	return (
		<Autocomplete
			sx={{ mt: '1rem', width: isMobile ? 200 : 400 }}
			onChange={(_, value) => getItem(value)}
			disablePortal
			value={item}
			getOptionLabel={getOptionLabel}
			options={equipmentType}
			slotProps={{ listbox: { sx: { maxHeight: '100px' } } }}
			renderInput={params => <TextField {...params} label='Type' />}
		/>
	)
}
