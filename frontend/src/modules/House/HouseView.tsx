import { lazy, Suspense, useEffect, useState } from 'react'
const HouseTable = lazy(() => import('./components/HouseTable'))
import { Container, useMediaQuery, useTheme } from '@mui/material'
import type { HouseClass } from './scripts/HouseClass'
import { houseApi } from '../../store/api/houseApi'
import { showAlert } from '../../store/application-store'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { store } from '../../store/store'
import { data, useLoaderData, Await } from 'react-router'
import type { GridSortModel, GridFilterModel } from '@mui/x-data-grid'

export default function HouseView() {
	const { houses } = useLoaderData() as { houses: HouseClass[] }

	const [sortModel, setSortModel] = useState<GridSortModel>([])
	const [filterModel, setFilterModel] = useState<GridFilterModel>({ items: [] })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		const savedSortModel = localStorage.getItem('houseTableSortModel')
		const savedFilterModel = localStorage.getItem('houseTableFilterModel')

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
				<Await resolve={houses}>
					{housesData => <HouseTable houses={housesData} initSort={sortModel} initFilter={filterModel} />}
				</Await>
			</Suspense>
		</Container>
	)
}

export async function loader(): Promise<{ houses: HouseClass[] }> {
	try {
		const promise = await store.dispatch(houseApi.endpoints.getHouses.initiate({})).unwrap()
		if (!promise) {
			throw data('Data not Found', { status: 404 })
		}
		return { houses: promise }
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
