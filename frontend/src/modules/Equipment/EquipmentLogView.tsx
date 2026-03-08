import { Suspense, lazy, useEffect } from 'react'
const EquipmentLogTable = lazy(() => import('./components/EquipmentLogTable'))
import type { EquipmentClass } from './scripts/EquipmentClass'
import { Container, useMediaQuery, useTheme } from '@mui/material'
import { equipmentApi } from '../../store/api/equipmentApi'
import { showAlert } from '../../store/application-store'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { store } from '../../store/store'
import { Await, useLoaderData, data, type LoaderFunctionArgs, useRevalidator } from 'react-router'
import { socket } from '../../socket/socket'

export default function EquipmentLogView() {
	const revalidator = useRevalidator()
	const { equipment, equLoggerId } = useLoaderData() as { equipment: Promise<EquipmentClass>; equLoggerId: number }

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (!equLoggerId) return

		const channel = `logger_${equLoggerId}`

		function onLastSeenRefreshEvent(): void {
			revalidator.revalidate()
		}

		socket.on(channel, onLastSeenRefreshEvent)
		return () => {
			socket.off(channel, onLastSeenRefreshEvent)
		}
	}, [revalidator, equLoggerId])

	return (
		<Container maxWidth={isMobile ? 'sm' : 'xl'}>
			<Suspense fallback={<LoadingCircle />}>
				<Await resolve={equipment}>
					{equipmentData => {
						return <EquipmentLogTable equipment={equipmentData} />
					}}
				</Await>
			</Suspense>
		</Container>
	)
}

export async function loader({ params }: LoaderFunctionArgs): Promise<{
	equipment: EquipmentClass
	equLoggerId: number
}> {
	try {
		if (!params.equLoggerId) {
			throw data('No logger Id', { status: 400 })
		}
		const promise = await store
			.dispatch(equipmentApi.endpoints.getEquipment.initiate(Number(params.equLoggerId), { forceRefetch: true }))
			.unwrap()
		if (!promise) {
			throw data('Data not Found', { status: 404 })
		}
		return { equipment: promise, equLoggerId: Number(params.equLoggerId) }
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
