import { Suspense, lazy, useEffect, useState } from 'react'
const EquipmentTable = lazy(() => import('./components/EquipmentTable'))
import type { EquipmentClass } from './scripts/EquipmentClass'
import { Container, useMediaQuery, useTheme } from '@mui/material'
import { equipmentApi } from '../../store/api/equipmentApi'
import { showAlert } from '../../store/application-store'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { store } from '../../store/store'
import { Await, useLoaderData, data } from 'react-router'
import type { GridSortModel, GridFilterModel } from '@mui/x-data-grid'

export default function EquipmentMainView() {
	const { equipments } = useLoaderData() as { equipments: Promise<EquipmentClass[]> }

	const [sortModel, setSortModel] = useState<GridSortModel>([])
	const [filterModel, setFilterModel] = useState<GridFilterModel>({ items: [] })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		const savedSortModel = localStorage.getItem('equipmentTableSortModel')
		const savedFilterModel = localStorage.getItem('equipmentTableFilterModel')

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
				<Await resolve={equipments}>
					{equipmentData => <EquipmentTable equipment={equipmentData} initSort={sortModel} initFilter={filterModel} />}
				</Await>
			</Suspense>
		</Container>
	)
}

export async function loader(): Promise<{ equipments: EquipmentClass[] }> {
	try {
		const promise = await store.dispatch(equipmentApi.endpoints.getEquipments.initiate({})).unwrap()
		if (!promise) {
			throw data('Data not Found', { status: 404 })
		}
		return { equipments: promise }
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
