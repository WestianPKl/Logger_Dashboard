import { useEffect } from 'react'
import type { HouseClass } from '../modules/House/scripts/HouseClass'
import { TextField, Autocomplete, useMediaQuery, useTheme } from '@mui/material'
import { useGetHousesQuery } from '../store/api/houseApi'
import { useAppDispatch } from '../store/hooks'
import { showAlert } from '../store/application-store'

interface ISelectProps {
	getItem: (item: HouseClass | null) => void
	item: HouseClass | null | undefined
	disabled?: boolean
}

export default function HouseSelect({ getItem, item, disabled }: ISelectProps) {
	const dispatch = useAppDispatch()

	const { data: house = [], error: houseError } = useGetHousesQuery({})

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (houseError) {
			const message = (houseError as any)?.data?.message || (houseError as any)?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [dispatch, houseError])

	function getOptionLabel(house: HouseClass): string {
		if (house.name && house.postalCode && house.city) {
			return `${house.name} ${house.postalCode} ${house.city}`
		}
		return house.name || ''
	}

	return (
		<Autocomplete
			sx={{ mt: '1rem', width: isMobile ? 200 : 400 }}
			onChange={(_, value) => getItem(value)}
			disablePortal
			value={item}
			getOptionLabel={getOptionLabel}
			options={house}
			disabled={disabled}
			slotProps={{ listbox: { sx: { maxHeight: '100px' } } }}
			renderInput={params => <TextField {...params} label='House' />}
		/>
	)
}
