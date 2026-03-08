import { useEffect } from 'react'
import type { HouseClass } from '../House/scripts/HouseClass'
import { Box, Grid, Typography } from '@mui/material'
import { socket } from '../../socket/socket'
import DashboardCard from './components/DashboardCard'
import { useRevalidator } from 'react-router'

export default function DashboardView({ houses }: { houses: HouseClass[] }) {
	const revalidator = useRevalidator()

	useEffect(() => {
		function onAddHouseEvent(): void {
			revalidator.revalidate()
		}

		socket.on('house', onAddHouseEvent)
		return () => {
			socket.off('house', onAddHouseEvent)
		}
	}, [revalidator])

	return (
		<>
			<Box sx={{ mb: 3 }}>
				<Typography variant='h5' sx={{ fontWeight: 700, color: 'text.primary' }}>
					Main dashboard
				</Typography>
				<Typography variant='body2' sx={{ color: 'text.secondary', mt: 0.5 }}>
					Overview of all your registered houses
				</Typography>
			</Box>
			<Box
				sx={{
					display: 'flex',
					flexDirection: 'column',
					justifyContent: 'center',
					alignContent: 'center',
					textAlign: 'center',
				}}>
				<Grid container spacing={{ xs: 2, md: 3 }} columns={{ xs: 2, sm: 8, md: 12 }}>
					{houses.map((ele: HouseClass) => (
						<DashboardCard key={ele.id} data={ele} />
					))}
				</Grid>
			</Box>
		</>
	)
}
