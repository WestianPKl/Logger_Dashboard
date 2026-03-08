import type { IDataMainProps } from '../scripts/IData'
import { useNavigate } from 'react-router'
import { lazy, Suspense, useMemo } from 'react'
const DataMainList = lazy(() => import('./DataMainList'))
import { Card, CardContent, CardActionArea, Typography, Box, Badge, List, ListItem } from '@mui/material'

export default function DataMain({ equipment, lastValues }: IDataMainProps) {
	const navigate = useNavigate()

	const lastValueData = useMemo(
		() => lastValues.filter(lv => lv.equLoggerId === equipment.id),
		[lastValues, equipment.id],
	)

	const isActive = useMemo(() => {
		if (!lastValueData.length) return false
		return lastValueData.some(e => {
			if (e.time) {
				const lastValueDate = new Date(e.time)
				const currentDate = new Date()
				return lastValueDate.getTime() - currentDate.getTime() > -1800000
			}
			return false
		})
	}, [lastValueData])

	function dataClickHandler(): void {
		navigate(`/data/data-logger/${equipment.id}`)
	}

	return (
		<Card sx={{ width: 320, maxWidth: 320 }} onClick={dataClickHandler}>
			<CardActionArea sx={{ width: '100%', height: '100%' }} onClick={dataClickHandler}>
				<CardContent sx={{ p: 2.5 }}>
					<Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 1.5 }}>
						<Typography variant='subtitle1' sx={{ fontWeight: 600 }}>
							{`ID${equipment.id} ${equipment.vendor?.name ?? ''} ${equipment.model?.name ?? ''}`}
						</Typography>
						<Badge color={isActive ? 'success' : 'error'} badgeContent=' ' variant='dot' />
					</Box>
					<Box sx={{ textAlign: 'left' }}>
						{lastValueData.length > 0 ? (
							<List sx={{ m: 0, p: 0 }}>
								{lastValueData.map(item => (
									<ListItem
										key={`${item.equLoggerId}-${item.parameter ?? item.id ?? Math.random()}`}
										sx={{ px: 0, py: 0.3 }}>
										<Suspense fallback={<div>Loading...</div>}>
											<DataMainList lastValue={item} />
										</Suspense>
									</ListItem>
								))}
							</List>
						) : (
							<Typography variant='body2' sx={{ color: 'text.secondary', py: 1 }}>
								No data
							</Typography>
						)}
					</Box>
				</CardContent>
			</CardActionArea>
		</Card>
	)
}
