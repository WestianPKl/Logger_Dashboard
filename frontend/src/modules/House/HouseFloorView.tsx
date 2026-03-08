import { lazy, Suspense, useEffect, useState } from 'react'
const HouseFloorTable = lazy(() => import('./components/HouseFloorTable'))
import { Container, useMediaQuery, useTheme } from '@mui/material'
import type { HouseFloorClass } from './scripts/HouseFloorClass'
import { houseApi } from '../../store/api/houseApi'
import { showAlert } from '../../store/application-store'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { data, useLoaderData, Await } from 'react-router'
import { store } from '../../store/store'
import type { GridSortModel, GridFilterModel } from '@mui/x-data-grid'

export default function HouseFloorView() {
	const { houseFloors } = useLoaderData() as { houseFloors: HouseFloorClass[] }

	const [sortModel, setSortModel] = useState<GridSortModel>([])
	const [filterModel, setFilterModel] = useState<GridFilterModel>({ items: [] })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		const savedSortModel = localStorage.getItem('houseFloorTableSortModel')
		const savedFilterModel = localStorage.getItem('houseFloorTableFilterModel')

		if (savedFilterModel) {
			try {
				const parsedFilterModel = JSON.parse(savedFilterModel)
				if (parsedFilterModel && typeof parsedFilterModel === 'object' && Array.isArray(parsedFilterModel.items)) {
					setFilterModel(parsedFilterModel)
				}
			} catch (err) {
				// pass
			}
		}

		if (savedSortModel) {
			try {
				const parsedSortModel = JSON.parse(savedSortModel)
				if (Array.isArray(parsedSortModel)) {
					setSortModel(parsedSortModel)
				}
			} catch (err) {
				// pass
			}
		}
	}, [])

	return (
		<Container maxWidth={isMobile ? 'sm' : 'xl'}>
			<Suspense fallback={<LoadingCircle />}>
				<Await resolve={houseFloors}>
					{houseFloorsData => (
						<HouseFloorTable houseFloors={houseFloorsData} initSort={sortModel} initFilter={filterModel} />
					)}
				</Await>
			</Suspense>
		</Container>
	)
}

export async function loader(): Promise<{ houseFloors: HouseFloorClass[] }> {
	try {
		const promise = await store.dispatch(houseApi.endpoints.getHouseFloors.initiate({})).unwrap()
		if (!promise) {
			throw data('Data not Found', { status: 404 })
		}
		return { houseFloors: promise }
	} catch (err: any) {
		store.dispatch(
			showAlert({
				message: err?.data?.message || err?.message || 'Something went wrong!',
				severity: 'error',
			}),
		)
		throw err
	}
}
