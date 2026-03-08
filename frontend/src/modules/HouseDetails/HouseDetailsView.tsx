import { useState, Suspense, useEffect } from 'react'
import { redirect, Await, useLoaderData, data, type LoaderFunctionArgs, useRevalidator, useParams } from 'react-router'
import { Tabs, Tab, Box, useMediaQuery, useTheme, Typography } from '@mui/material'
import { HouseClass } from '../House/scripts/HouseClass'
import { houseApi } from '../../store/api/houseApi'
import { canWrite } from '../../store/auth-actions'
import { useAppSelector } from '../../store/hooks'
import TabPanel, { tabProps } from '../../components/UI/TabPanel'
import HouseDetailsFloor from './components/HouseDetailsFloor'
import HouseDetailsMobile from './components/HouseDetailsMobile'
import HouseEditView from './HouseEditView'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { store } from '../../store/store'
import { showAlert } from '../../store/application-store'
import { socket } from '../../socket/socket'

export default function HouseDetailsView() {
	const { house } = useLoaderData() as { house: HouseClass }
	const { houseId } = useParams<{ houseId: string }>()
	const revalidator = useRevalidator()
	const [value, setValue] = useState(0)

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('md'))
	const isWritable = useAppSelector(state => canWrite('house', 'houseHouse')(state))

	useEffect(() => {
		if (!houseId) return

		const channel = `house-${houseId}`

		function onHouseEvent() {
			revalidator.revalidate()
		}

		socket.on(channel, onHouseEvent)

		return () => {
			socket.off(channel, onHouseEvent)
		}
	}, [houseId, revalidator])

	return (
		<Box
			component='div'
			sx={{
				display: 'flex',
				flexDirection: 'column',
				justifyContent: 'center',
				alignContent: 'center',
				textAlign: 'center',
			}}>
			<Suspense fallback={<LoadingCircle />}>
				<Await resolve={house}>
					{houseData => (
						<>
							{houseData && houseData.floors.length > 0 && (
								<>
									<Box component='div' sx={{ borderBottom: 1, borderColor: 'divider' }}>
										<Tabs
											value={value}
											onChange={(_: React.SyntheticEvent, newValue: number) => setValue(newValue)}
											aria-label='Horizontal tab House Details'>
											{houseData.floors &&
												houseData.floors.map((e, index) =>
													e.id ? <Tab key={e.id} label={e.name} {...tabProps(index)} /> : null,
												)}
											{isWritable && <Tab label={'House'} {...tabProps(houseData.floors.length)} />}
										</Tabs>
									</Box>
									<Box component='div' sx={{ textAlign: 'center', justifyContent: 'center' }}>
										{houseData.floors.map(
											(e, index) =>
												e.id && (
													<TabPanel key={e.id} value={value} index={index}>
														{!isMobile ? (
															<HouseDetailsFloor floor={e} houseId={houseData.id} />
														) : (
															<HouseDetailsMobile floor={e} />
														)}
													</TabPanel>
												),
										)}
										{isWritable && (
											<TabPanel value={value} index={houseData.floors.length}>
												<HouseEditView data={houseData} />
											</TabPanel>
										)}
									</Box>
								</>
							)}
							{houseData && houseData.floors.length === 0 && (
								<Box sx={{ mt: 2, textAlign: 'center' }}>
									<Typography variant='body1' color='text.secondary'>
										Please add floors
									</Typography>
								</Box>
							)}
						</>
					)}
				</Await>
			</Suspense>
		</Box>
	)
}

export async function loader({ params }: LoaderFunctionArgs): Promise<Response | { house: HouseClass }> {
	if (!params.houseId) throw data('No house Id', { status: 400 })

	if (!localStorage.getItem('token')) {
		store.dispatch(showAlert({ message: 'Unknown user - token not found', severity: 'error' }))
		return redirect('/login')
	}

	try {
		const house = await store
			.dispatch(houseApi.endpoints.getHouse.initiate(parseInt(params.houseId), { forceRefetch: true }))
			.unwrap()

		if (!house) throw data('Data not Found', { status: 404 })

		return { house }
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
