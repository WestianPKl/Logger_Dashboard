import CircularProgress from '@mui/material/CircularProgress'
import Box from '@mui/material/Box'

export default function LoadingCircle() {
	return (
		<Box
			sx={{
				mt: 6,
				minHeight: 200,
				display: 'flex',
				flexDirection: 'column',
				gap: 2,
				alignItems: 'center',
				justifyContent: 'center',
			}}>
			<CircularProgress size={40} thickness={4} />
		</Box>
	)
}
