import { lazy, Suspense, useEffect, useState } from 'react'
const AdminFunctionalityDefinitionTable = lazy(() => import('./components/AdminFunctionalityDefinitionTable'))
import type { FunctionalityDefinitionClass } from './scripts/FunctionalityDefinitionClass'
import { Container, useMediaQuery, useTheme } from '@mui/material'
import { adminApi } from '../../store/api/adminApi'
import { showAlert } from '../../store/application-store'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { store } from '../../store/store'
import { data, useLoaderData, Await } from 'react-router'
import type { GridSortModel, GridFilterModel } from '@mui/x-data-grid'

export default function AdminFunctionalityDefinitionView() {
	const { functionalityDefinitions } = useLoaderData() as { functionalityDefinitions: FunctionalityDefinitionClass[] }

	const [sortModel, setSortModel] = useState<GridSortModel>([])
	const [filterModel, setFilterModel] = useState<GridFilterModel>({ items: [] })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		const savedSortModel = localStorage.getItem('functionalityDefinitionTableSortModel')
		const savedFilterModel = localStorage.getItem('functionalityDefinitionTableFilterModel')

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
				<Await resolve={functionalityDefinitions}>
					{functionalityDefinitionsData => (
						<AdminFunctionalityDefinitionTable
							functionalityDefinitions={functionalityDefinitionsData}
							initSort={sortModel}
							initFilter={filterModel}
						/>
					)}
				</Await>
			</Suspense>
		</Container>
	)
}

export async function loader(): Promise<{ functionalityDefinitions: FunctionalityDefinitionClass[] }> {
	try {
		const promise = await store.dispatch(adminApi.endpoints.getFunctionalityDefinitions.initiate({})).unwrap()
		if (!promise) {
			throw data('Data not Found', { status: 404 })
		}
		return { functionalityDefinitions: promise }
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
