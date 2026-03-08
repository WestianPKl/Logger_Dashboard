import { Box, Container, useMediaQuery, useTheme, Grid, Typography } from '@mui/material'
import { lazy, Suspense } from 'react'
import type { EquipmentClass } from '../Equipment/scripts/EquipmentClass'
import { showAlert } from '../../store/application-store'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { Await, data, useLoaderData } from 'react-router'
import { store } from '../../store/store'
import { equipmentApi } from '../../store/api/equipmentApi'
import { dataApi } from '../../store/api/dataApi'
import type { DataLastValueViewClass } from './scripts/DataLastValueViewClass'
const DataMain = lazy(() => import('./components/DataMain'))

export default function DataMainView() {
	const { equipments, lastValues } = useLoaderData() as {
		equipments: EquipmentClass[]
		lastValues: DataLastValueViewClass[]
	}

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	return (
		<Container maxWidth={isMobile ? 'sm' : 'xl'} sx={{ textAlign: 'center' }}>
			<Box>
				<Suspense fallback={<LoadingCircle />}>
					<Grid
						container
						sx={{
							display: 'flex',
							flexDirection: 'column',
							justifyContent: 'center',
							alignContent: 'center',
							textAlign: 'center',
						}}
						spacing={{ xs: 2, md: 3 }}
						columns={{ xs: 2, sm: 8, md: 12 }}>
						<Await resolve={equipments}>
							{equipmentsData => (
								<>
									{equipmentsData.length === 0 && (
										<Typography variant='h6' sx={{ m: 2 }}>
											Brak dostępnych urządzeń typu Logger
										</Typography>
									)}
									{equipmentsData.length > 0 &&
										equipmentsData.map((item: EquipmentClass) => (
											<DataMain key={item.id} equipment={item} lastValues={lastValues} />
										))}
								</>
							)}
						</Await>
					</Grid>
				</Suspense>
			</Box>
		</Container>
	)
}

export async function loader(): Promise<{ equipments: EquipmentClass[]; lastValues: DataLastValueViewClass[] }> {
	try {
		const equType = await store.dispatch(equipmentApi.endpoints.getEquipmentTypes.initiate({ name: 'Logger' })).unwrap()
		if (!equType) {
			throw data('Data not Found', { status: 404 })
		}
		const equipments = await store
			.dispatch(equipmentApi.endpoints.getEquipments.initiate({ equTypeId: equType[0].id }))
			.unwrap()
		if (!equipments) {
			throw data('Data not Found', { status: 404 })
		}
		const loggerIds = equipments.map((eq: EquipmentClass) => eq.id)
		const lastValues = await store.dispatch(dataApi.endpoints.getDataLastValuesMulti.initiate({ loggerIds })).unwrap()
		return { equipments, lastValues }
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
