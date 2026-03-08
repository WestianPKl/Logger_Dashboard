import type { IHouseEditViewProps } from './scripts/IHouseDetails'
import { Container, useMediaQuery, useTheme } from '@mui/material'
import HouseEditForm from './components/HouseEditForm'
import { useAppDispatch } from '../../store/hooks'
import type { IAddHouseData, IAddHouseFloorData } from '../House/scripts/IHouse'
import { showAlert } from '../../store/application-store'
import { useUpdateHouseMutation, useAddHouseFloorMutation } from '../../store/api/houseApi'
import { useRevalidator } from 'react-router'

export default function HouseEditView({ data }: IHouseEditViewProps) {
	const dispatch = useAppDispatch()
	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))
	const revalidator = useRevalidator()

	const [updateHouse] = useUpdateHouseMutation()
	const [addHouseFloor] = useAddHouseFloorMutation()

	async function addHouseFloorHandler(item: IAddHouseFloorData[] | IAddHouseFloorData): Promise<void> {
		const itemsArr = Array.isArray(item) ? item : [item]
		try {
			await Promise.all(
				itemsArr.map(async i => {
					const formData = new FormData()
					if (i.name) formData.append('name', i.name)
					if (i.houseId) formData.append('houseId', `${i.houseId}`)
					if (i.layout) formData.append('layout', i.layout)
					await addHouseFloor(formData).unwrap()
				}),
			)
			dispatch(showAlert({ message: 'New house floor(s) added', severity: 'success' }))
			revalidator.revalidate()
		} catch (err: any) {
			const message = err?.data?.message || err?.message || 'Error adding floor'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}

	async function editHouseHandler(item: IAddHouseData[] | IAddHouseData): Promise<void> {
		const itemsArr = Array.isArray(item) ? item : [item]
		try {
			await Promise.all(
				itemsArr.map(async i => {
					const formData = new FormData()
					if (i.name) formData.append('name', i.name)
					if (i.postalCode) formData.append('postalCode', i.postalCode)
					if (i.city) formData.append('city', i.city)
					if (i.street) formData.append('street', i.street)
					if (i.houseNumber) formData.append('houseNumber', i.houseNumber)
					if (i.pictureLink) formData.append('pictureLink', i.pictureLink)
					if (i.id) {
						await updateHouse({ body: formData, id: i.id }).unwrap()
					}
				}),
			)
			dispatch(showAlert({ message: 'House(s) edited', severity: 'success' }))
			revalidator.revalidate()
		} catch (err: any) {
			const message = err?.data?.message || err?.message || 'Error editing house'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}

	return (
		<Container maxWidth={isMobile ? 'sm' : 'xl'} sx={{ textAlign: 'center' }}>
			<HouseEditForm house={data} addHouseFloorHandler={addHouseFloorHandler} editHouseHandler={editHouseHandler} />
		</Container>
	)
}
