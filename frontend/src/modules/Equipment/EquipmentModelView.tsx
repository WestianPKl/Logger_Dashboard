import { Suspense, lazy, useEffect, useState } from 'react'
const EquipmentModelTable = lazy(() => import('./components/EquipmentModelTable'))
import { Container, useMediaQuery, useTheme } from '@mui/material'
import type { EquipmentModelClass } from './scripts/EquipmentModelClass'
import { equipmentApi } from '../../store/api/equipmentApi'
import { showAlert } from '../../store/application-store'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { store } from '../../store/store'
import { redirect, useLoaderData, Await, data } from 'react-router'
import type { GridSortModel, GridFilterModel } from '@mui/x-data-grid'

export default function EquipmentModelView() {
	const { equipmentModels } = useLoaderData() as { equipmentModels: Promise<EquipmentModelClass[]> }

	const [sortModel, setSortModel] = useState<GridSortModel>([])
	const [filterModel, setFilterModel] = useState<GridFilterModel>({ items: [] })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		const savedSortModel = localStorage.getItem('equipmentModelTableSortModel')
		const savedFilterModel = localStorage.getItem('equipmentModelTableFilterModel')

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
				<Await resolve={equipmentModels}>
					{equipmentModelsData => (
						<EquipmentModelTable equipmentModel={equipmentModelsData} initSort={sortModel} initFilter={filterModel} />
					)}
				</Await>
			</Suspense>
		</Container>
	)
}

export async function loader(): Promise<Response | { equipmentModels: EquipmentModelClass[] }> {
	if (!localStorage.getItem('token')) {
		return redirect('/login')
	}
	try {
		const promise = await store.dispatch(equipmentApi.endpoints.getEquipmentModels.initiate({})).unwrap()
		if (!promise) {
			throw data('Data not Found', { status: 404 })
		}
		return { equipmentModels: promise }
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
