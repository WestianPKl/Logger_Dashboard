import { useNavigate } from 'react-router'
import {
	Card,
	CardContent,
	Typography,
	CardActionArea,
	CardMedia,
	Box,
	Grid,
	useMediaQuery,
	useTheme,
} from '@mui/material'
import type { IDashboardCardProps } from '../scripts/IDashboard'

const fallbackImage = '/img/house-placeholder.webp'

export default function DashboardCard({ data }: IDashboardCardProps) {
	const navigate = useNavigate()
	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	function houseClickHandler(id?: number): void {
		if (id) {
			navigate(`/house-details/${id}`)
		}
	}

	return (
		<Grid size={{ xs: 2, sm: 4, md: 4 }} key={data.id}>
			<Card
				sx={{
					minWidth: isMobile ? 200 : 280,
					maxWidth: 400,
					height: 360,
					overflow: 'hidden',
				}}>
				<CardActionArea sx={{ width: '100%', height: '100%' }} onClick={() => houseClickHandler(data.id)}>
					<Box sx={{ position: 'relative', overflow: 'hidden' }}>
						<CardMedia
							component='img'
							height='200'
							loading='lazy'
							image={
								data.pictureLink
									? `${import.meta.env.VITE_API_IP}/${data.pictureLink}?w=200&h=200&format=webp`
									: fallbackImage
							}
							alt={`House ${data.name || ''} picture`}
							sx={{ transition: 'transform 0.4s ease', '&:hover': { transform: 'scale(1.05)' } }}
						/>
						<Box
							sx={{
								position: 'absolute',
								bottom: 0,
								left: 0,
								right: 0,
								height: '50%',
								background: 'linear-gradient(to top, rgba(0,0,0,0.35), transparent)',
								pointerEvents: 'none',
							}}
						/>
					</Box>
					<CardContent sx={{ px: 2.5, py: 2 }}>
						<Typography gutterBottom variant='h6' component='div' sx={{ fontWeight: 600, mb: 0.5 }}>
							{data.name || 'Unknown'}
						</Typography>
						<Box>
							<Typography variant='body2' sx={{ color: 'text.secondary' }}>
								{`${data.postalCode || ''} ${data.city || ''}`}
							</Typography>
							<Typography variant='body2' sx={{ color: 'text.secondary' }}>
								{`${data.street || ''} ${data.houseNumber || ''}`}
							</Typography>
						</Box>
					</CardContent>
				</CardActionArea>
			</Card>
		</Grid>
	)
}
