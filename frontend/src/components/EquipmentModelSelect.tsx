import { useEffect } from 'react'
import type { EquipmentModelClass } from '../modules/Equipment/scripts/EquipmentModelClass'
import { TextField, Autocomplete, useMediaQuery, useTheme } from '@mui/material'
import { useGetEquipmentModelsQuery } from '../store/api/equipmentApi'
import { useAppDispatch } from '../store/hooks'
import { showAlert } from '../store/application-store'

interface ISelectProps {
	getItem: (item: EquipmentModelClass | null) => void
	item: EquipmentModelClass | null | undefined
}

export default function EquipmentModelSelect({ getItem, item }: ISelectProps) {
	const dispatch = useAppDispatch()

	const { data: equipmentModel = [], error: equipmentModelError } = useGetEquipmentModelsQuery({})

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (equipmentModelError) {
			const message =
				(equipmentModelError as any)?.data?.message || (equipmentModelError as any)?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [dispatch, equipmentModelError])

	function getOptionLabel(model: EquipmentModelClass): string {
		return model.name || ''
	}

	return (
		<Autocomplete
			sx={{ mt: '1rem', width: isMobile ? 200 : 400 }}
			onChange={(_, value) => getItem(value)}
			disablePortal
			value={item}
			getOptionLabel={getOptionLabel}
			options={equipmentModel}
			slotProps={{ listbox: { sx: { maxHeight: '100px' } } }}
			renderInput={params => <TextField {...params} label='Model' />}
		/>
	)
}
