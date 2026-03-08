import { useEffect } from 'react'
import type { HouseFloorClass } from '../modules/House/scripts/HouseFloorClass'
import { TextField, Autocomplete, useMediaQuery, useTheme } from '@mui/material'
import { useGetHouseFloorsQuery } from '../store/api/houseApi'
import { useAppDispatch } from '../store/hooks'
import { showAlert } from '../store/application-store'

interface ISelectProps {
	getItem: (item: HouseFloorClass | null) => void
	item: HouseFloorClass | null | undefined
}

export default function HouseFloorSelect({ getItem, item }: ISelectProps) {
	const dispatch = useAppDispatch()

	const { data: houseFloor = [], error: houseFloorError } = useGetHouseFloorsQuery({})

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (houseFloorError) {
			const message =
				(houseFloorError as any)?.data?.message || (houseFloorError as any)?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [dispatch, houseFloorError])

	function getOptionLabel(houseFloor: HouseFloorClass): string {
		return houseFloor.name || ''
	}

	return (
		<Autocomplete
			sx={{ mt: '1rem', width: isMobile ? 200 : 400 }}
			onChange={(_, value) => getItem(value)}
			disablePortal
			value={item}
			getOptionLabel={getOptionLabel}
			options={houseFloor}
			slotProps={{ listbox: { sx: { maxHeight: '100px' } } }}
			renderInput={params => <TextField {...params} label='Floor' />}
		/>
	)
}
