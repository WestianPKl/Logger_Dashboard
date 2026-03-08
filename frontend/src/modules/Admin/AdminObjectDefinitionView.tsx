import { lazy, Suspense, useEffect, useState } from 'react'
import type { GridSortModel, GridFilterModel } from '@mui/x-data-grid'
const AdminObjectDefinitionTable = lazy(() => import('./components/AdminObjectDefinitionTable'))
import type { ObjectDefinitionClass } from './scripts/ObjectDefinitionClass'
import { Container, useMediaQuery, useTheme } from '@mui/material'
import { adminApi } from '../../store/api/adminApi'
import { showAlert } from '../../store/application-store'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { store } from '../../store/store'
import { data, useLoaderData, Await } from 'react-router'

export default function AdminObjectDefinitionView() {
	const { objectDefinitions } = useLoaderData() as { objectDefinitions: ObjectDefinitionClass[] }

	const [sortModel, setSortModel] = useState<GridSortModel>([])
	const [filterModel, setFilterModel] = useState<GridFilterModel>({ items: [] })

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		const savedSortModel = localStorage.getItem('objectDefinitionTableSortModel')
		const savedFilterModel = localStorage.getItem('objectDefinitionTableFilterModel')

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
				<Await resolve={objectDefinitions}>
					{objectDefinitionsData => (
						<AdminObjectDefinitionTable
							objectDefinitions={objectDefinitionsData}
							initSort={sortModel}
							initFilter={filterModel}
						/>
					)}
				</Await>
			</Suspense>
		</Container>
	)
}

export async function loader(): Promise<{ objectDefinitions: ObjectDefinitionClass[] }> {
	try {
		const promise = await store.dispatch(adminApi.endpoints.getObjectDefinitions.initiate({})).unwrap()
		if (!promise) {
			throw data('Data not Found', { status: 404 })
		}
		return { objectDefinitions: promise }
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
