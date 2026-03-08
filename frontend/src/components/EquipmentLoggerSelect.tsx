import { useEffect } from 'react'
import type { EquipmentClass } from '../modules/Equipment/scripts/EquipmentClass'
import { TextField, Autocomplete, useMediaQuery, useTheme } from '@mui/material'
import { useGetEquipmentsQuery, useGetEquipmentUnusedLoggersQuery } from '../store/api/equipmentApi'
import { useAppDispatch } from '../store/hooks'
import { showAlert } from '../store/application-store'
import type { EquipmentUnusedLoggerClass } from '../modules/Equipment/scripts/EquipmentUnusedLoggerClass'

interface ISelectProps {
	getItem: (item: EquipmentClass | null) => void
	item: EquipmentClass | null | undefined
	disabled?: boolean
}

export default function EquipmentLoggerSelect({ getItem, item, disabled }: ISelectProps) {
	const dispatch = useAppDispatch()

	const { data: unusedLoggers = [], error: unusedLoggersError } = useGetEquipmentUnusedLoggersQuery({})
	const loggersIds = unusedLoggers
		.map((e: EquipmentUnusedLoggerClass) => e.equLoggerId)
		.filter((id): id is number => typeof id === 'number')
	const skip = loggersIds.length === 0
	const { data: equipment = [], error: equipmentsError } = useGetEquipmentsQuery(loggersIds, { skip })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (unusedLoggersError || equipmentsError) {
			const err = unusedLoggersError || equipmentsError
			const message = (err as any)?.data?.message || (err as any)?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [unusedLoggersError, equipmentsError, dispatch])

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
			options={equipment}
			disabled={disabled}
			slotProps={{ listbox: { sx: { maxHeight: '100px' } } }}
			renderInput={params => <TextField {...params} label='Logger' />}
		/>
	)
}
