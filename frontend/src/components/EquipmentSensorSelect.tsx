import { useEffect } from 'react'
import type { EquipmentClass } from '../modules/Equipment/scripts/EquipmentClass'
import { TextField, Autocomplete, useMediaQuery, useTheme } from '@mui/material'
import { useGetEquipmentsQuery, useGetEquipmentTypesQuery } from '../store/api/equipmentApi'
import { useAppDispatch } from '../store/hooks'
import { showAlert } from '../store/application-store'

interface ISelectProps {
	getItem: (item: EquipmentClass | null) => void
	item: EquipmentClass | null | undefined
}

export default function EquipmentSensorSelect({ getItem, item }: ISelectProps) {
	const dispatch = useAppDispatch()

	const { data: equipmentType = [], error: equipmentTypeError } = useGetEquipmentTypesQuery({ name: 'Sensor' })
	const sensorTypeIds = equipmentType[0]
	const skip = !sensorTypeIds
	const { data: equipment = [], error: equipmentsError } = useGetEquipmentsQuery({ equTypeId: sensorTypeIds }, { skip })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (equipmentTypeError || equipmentsError) {
			const err = equipmentTypeError || equipmentsError
			const message = (err as any)?.data?.message || (err as any)?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [equipmentTypeError, equipmentsError, dispatch])

	function getOptionLabel(equipment: EquipmentClass): string {
		return equipment ? `ID${equipment.id} ${equipment.vendor?.name} ${equipment.model?.name}` : ''
	}

	return (
		<Autocomplete
			sx={{ width: isMobile ? 200 : 400 }}
			onChange={(_, value) => getItem(value)}
			disablePortal
			value={item}
			getOptionLabel={getOptionLabel}
			slotProps={{ listbox: { sx: { maxHeight: '100px' } } }}
			options={equipment}
			renderInput={params => <TextField {...params} label='Equipment' />}
		/>
	)
}
