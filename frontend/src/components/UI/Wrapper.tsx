import { Box } from '@mui/material'
import type { IPropsComponentsWrapper } from '../scripts/ComponentsInterface'

export default function Wrapper(props: IPropsComponentsWrapper) {
	return (
		<Box
			component={'main'}
			sx={{
				mt: { xs: 2, sm: 3 },
				mb: 4,
				px: { xs: 1.5, sm: 3, md: 4 },
				display: 'flex',
				flexDirection: 'column',
				flexWrap: 'nowrap',
				width: '100%',
				maxWidth: 1440,
				mx: 'auto',
			}}>
			{props.children}
		</Box>
	)
}
