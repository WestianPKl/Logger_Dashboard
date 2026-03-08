import { useEffect } from 'react'
import type { EquipmentVendorClass } from '../modules/Equipment/scripts/EquipmentVendorClass'
import { TextField, Autocomplete, useMediaQuery, useTheme } from '@mui/material'
import { useGetEquipmentVendorsQuery } from '../store/api/equipmentApi'
import { useAppDispatch } from '../store/hooks'
import { showAlert } from '../store/application-store'

interface ISelectProps {
	getItem: (item: EquipmentVendorClass | null) => void
	item: EquipmentVendorClass | null | undefined
}

export default function EquipmentVendorSelect({ getItem, item }: ISelectProps) {
	const dispatch = useAppDispatch()

	const { data: equipmentVendor = [], error: equipmentVendorError } = useGetEquipmentVendorsQuery({})

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (equipmentVendorError) {
			const message =
				(equipmentVendorError as any)?.data?.message || (equipmentVendorError as any)?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [dispatch, equipmentVendorError])

	function getOptionLabel(vendor: EquipmentVendorClass): string {
		return vendor.name || ''
	}

	return (
		<Autocomplete
			sx={{ mt: '1rem', width: isMobile ? 200 : 400 }}
			onChange={(_, value) => getItem(value)}
			disablePortal
			value={item}
			getOptionLabel={getOptionLabel}
			options={equipmentVendor}
			slotProps={{ listbox: { sx: { maxHeight: '100px' } } }}
			renderInput={params => <TextField {...params} label='Vendor' />}
		/>
	)
}
