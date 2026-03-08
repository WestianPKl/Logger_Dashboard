import { useRouteError } from 'react-router'
import { Box, Typography, Paper, Container } from '@mui/material'
import Wrapper from '../../components/UI/Wrapper'
import AppBarView from './AppBarView'
import ErrorOutlineIcon from '@mui/icons-material/ErrorOutline'

export default function ErrorView() {
	const error: any = useRouteError()

	let title = 'An error occurred!'
	let message = 'Something went wrong!'

	if (error.status === 500) {
		title = 'Internal Server Error'
		message = error.data.message
	}

	if (error.status === 404) {
		title = 'Not found!'
		message = 'Could not find resource or page.'
	}

	if (error.status === 401) {
		title = 'Unauthorized!'
		message = 'User unauthorized!'
	}

	if (error.status === 422) {
		title = 'Wrong input data'
		message = 'Wrong input data!'
	}

	return (
		<Box component={'section'}>
			<AppBarView />
			<Wrapper>
				<Container maxWidth='sm' sx={{ mt: { xs: 4, sm: 8 } }}>
					<Paper
						elevation={0}
						sx={{
							p: { xs: 4, sm: 6 },
							textAlign: 'center',
							border: '1px solid',
							borderColor: 'divider',
						}}>
						<ErrorOutlineIcon sx={{ fontSize: 64, color: 'error.main', mb: 2 }} />
						<Typography variant='h4' sx={{ fontWeight: 700, mb: 1 }}>
							{title}
						</Typography>
						<Typography variant='body1' sx={{ color: 'text.secondary' }}>
							{message}
						</Typography>
					</Paper>
				</Container>
			</Wrapper>
		</Box>
	)
}
