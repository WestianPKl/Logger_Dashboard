import { Suspense, lazy, useEffect, useState } from 'react'
import type { GridSortModel, GridFilterModel } from '@mui/x-data-grid'
const DataDefinitionTable = lazy(() => import('./components/DataDefinitionTable'))
import { Container, useMediaQuery, useTheme } from '@mui/material'
import { dataApi } from '../../store/api/dataApi'
import { showAlert } from '../../store/application-store'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { store } from '../../store/store'
import { redirect, Await, useLoaderData, data } from 'react-router'
import type { DataDefinitionClass } from './scripts/DataDefinitionClass'

export default function DataDefinitionView() {
	const { dataDefinitions } = useLoaderData() as { dataDefinitions: Promise<DataDefinitionClass[]> }

	const [sortModel, setSortModel] = useState<GridSortModel>([])
	const [filterModel, setFilterModel] = useState<GridFilterModel>({ items: [] })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		const savedSortModel = localStorage.getItem('dataDefinitionTableSortModel')
		const savedFilterModel = localStorage.getItem('dataDefinitionTableFilterModel')

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
				<Await resolve={dataDefinitions}>
					{dataDefinitionsData => (
						<DataDefinitionTable dataDefinitions={dataDefinitionsData} initSort={sortModel} initFilter={filterModel} />
					)}
				</Await>
			</Suspense>
		</Container>
	)
}

export async function loader(): Promise<Response | { dataDefinitions: DataDefinitionClass[] }> {
	if (!localStorage.getItem('token')) {
		return redirect('/login')
	}
	try {
		const promise = await store.dispatch(dataApi.endpoints.getDataDefinitions.initiate({})).unwrap()
		if (!promise) {
			throw data('Data not Found', { status: 404 })
		}
		return { dataDefinitions: promise }
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
