import { Link } from 'react-router'
import { Typography, Button, Paper, Container } from '@mui/material'
import SearchOffIcon from '@mui/icons-material/SearchOff'

export default function NotFoundView() {
	return (
		<Container maxWidth='sm' sx={{ mt: { xs: 4, sm: 8 } }}>
			<Paper
				elevation={0}
				sx={{
					p: { xs: 4, sm: 6 },
					textAlign: 'center',
					border: '1px solid',
					borderColor: 'divider',
				}}>
				<SearchOffIcon sx={{ fontSize: 64, color: 'text.secondary', mb: 2, opacity: 0.6 }} />
				<Typography variant='h2' sx={{ fontWeight: 800, color: 'primary.main', mb: 1 }}>
					404
				</Typography>
				<Typography variant='h6' sx={{ mb: 1, fontWeight: 600 }}>
					Page not found
				</Typography>
				<Typography variant='body2' sx={{ color: 'text.secondary', mb: 3 }}>
					The page you are looking for doesn't exist or has been moved.
				</Typography>
				<Button component={Link} to='/' variant='contained' size='large'>
					Return to Home
				</Button>
			</Paper>
		</Container>
	)
}
