import React, { useState, useEffect, lazy, Suspense } from 'react'
import { Box, Tabs, Tab, Typography } from '@mui/material'
import { DataConnectedSensorViewClass } from './scripts/DataConnectedSensorViewClass'
import { socket } from '../../socket/socket'
import LoadingCircle from '../../components/UI/LoadingCircle'
const DataChart = lazy(() => import('./components/DataChart'))
import TabPanel, { tabProps } from '../../components/UI/TabPanel'
import { EquipmentClass } from '../Equipment/scripts/EquipmentClass'
import { showAlert } from '../../store/application-store'
import { Await, data, useLoaderData, useRevalidator, type LoaderFunctionArgs } from 'react-router'
import { store } from '../../store/store'
import { dataApi } from '../../store/api/dataApi'
import { equipmentApi } from '../../store/api/equipmentApi'

export default function DataChartView() {
	const { equipments, connectedSensors, equLoggerId } = useLoaderData() as {
		equipments: EquipmentClass[]
		connectedSensors: DataConnectedSensorViewClass[]
		equLoggerId: number
	}

	const [value, setValue] = useState(0)
	const revalidator = useRevalidator()
	const handleChange = (_: React.SyntheticEvent, newValue: number) => {
		setValue(newValue)
	}

	useEffect(() => {
		function onRefreshDataEvent() {
			revalidator.revalidate()
		}

		socket.on(`loggerData_${equLoggerId}`, onRefreshDataEvent)

		return () => {
			socket.off(`loggerData_${equLoggerId}`, onRefreshDataEvent)
		}
	}, [revalidator, equLoggerId])

	return (
		<Suspense fallback={<LoadingCircle />}>
			<Await resolve={{ equipments, connectedSensors }}>
				{({ equipments: equipmentsData, connectedSensors: connectedSensorsData }) => (
					<>
						{connectedSensorsData.length > 0 && (
							<>
								<Box sx={{ marginLeft: 1, borderBottom: 1, borderColor: 'divider' }}>
									<Typography variant='body1' sx={{ m: 0, p: 0 }}>
										Logger ID{equLoggerId} S/N: {equipmentsData[0]?.serialNumber} {equipmentsData[0]?.vendor?.name}{' '}
										{equipmentsData[0]?.model?.name}
									</Typography>
									<Tabs
										value={value}
										onChange={handleChange}
										aria-label='Horizontal tabs House Main View'
										variant='scrollable'
										scrollButtons='auto'>
										{connectedSensorsData.map(
											(sensor, index) =>
												sensor.equSensorId && (
													<Tab
														label={`Sensor ID${sensor.equSensorId}`}
														{...tabProps(0 + index)}
														key={sensor.equSensorId}
													/>
												),
										)}
									</Tabs>
								</Box>

								<Box component='div' sx={{ width: '100%', textAlign: 'center', justifyContent: 'center' }}>
									{connectedSensorsData.map(
										(sensor, index) =>
											sensor.equSensorId && (
												<TabPanel value={value} index={0 + index} key={sensor.equSensorId}>
													<DataChart equLoggerId={equLoggerId} equSensorId={sensor.equSensorId} />
												</TabPanel>
											),
									)}
								</Box>
							</>
						)}
						{connectedSensorsData.length === 0 && (
							<Box sx={{ width: '100%', textAlign: 'center' }}>
								<p>No sensors connected - data could not be displayed</p>
							</Box>
						)}
					</>
				)}
			</Await>
		</Suspense>
	)
}

export async function loader({ params }: LoaderFunctionArgs): Promise<{
	equipments: EquipmentClass[]
	connectedSensors: DataConnectedSensorViewClass[]
	equLoggerId: number
}> {
	if (!params.equLoggerId) {
		throw data('No logger Id', { status: 400 })
	}
	try {
		const connectedSensors = await store
			.dispatch(
				dataApi.endpoints.getDataConnectedSensorView.initiate(
					{ equLoggerId: params.equLoggerId },
					{ forceRefetch: true },
				),
			)
			.unwrap()

		const equipments = await store
			.dispatch(equipmentApi.endpoints.getEquipments.initiate({ id: params.equLoggerId }, { forceRefetch: true }))
			.unwrap()
		if (!equipments || !connectedSensors) {
			throw data('Data not Found', { status: 404 })
		}
		return {
			equipments: equipments,
			connectedSensors: connectedSensors,
			equLoggerId: Number(params.equLoggerId),
		}
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
