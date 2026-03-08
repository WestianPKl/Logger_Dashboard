import { Button, Stack, useMediaQuery, useTheme } from '@mui/material'
import type { IDataChartRangeButtonsProps } from '../scripts/IData'

export default function DataChartRangeButtons({ range, handleRangeChange, handleReset }: IDataChartRangeButtonsProps) {
	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	return (
		<Stack
			direction={isMobile ? 'column' : 'row'}
			spacing={isMobile ? 0 : 2}
			flexWrap='wrap'
			alignItems='center'
			mb={2}>
			{[
				{ label: '1h', name: 'Last hour' },
				{ label: '1d', name: 'Last day' },
				{ label: '1w', name: 'Last week' },
				{ label: '1m', name: 'Last month' },
				{ label: 'all', name: 'All data' },
			].map((r: any) => (
				<Button
					sx={{ mb: isMobile ? 1 : 0 }}
					size={isMobile ? 'small' : 'medium'}
					key={r.label}
					variant={range === r.label ? 'contained' : 'outlined'}
					onClick={() => handleRangeChange(r.label)}>
					{r.name}
				</Button>
			))}
			<Button
				sx={{ mb: isMobile ? 1 : 0 }}
				size={isMobile ? 'small' : 'medium'}
				variant='outlined'
				color='error'
				onClick={handleReset}>
				Reset
			</Button>
		</Stack>
	)
}
