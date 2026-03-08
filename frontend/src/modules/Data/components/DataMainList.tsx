import { ListItemText, Typography, useMediaQuery, useTheme, ListItemAvatar, Tooltip } from '@mui/material'
import WaterDropIcon from '@mui/icons-material/WaterDrop'
import SpeedIcon from '@mui/icons-material/Speed'
import ThermostatIcon from '@mui/icons-material/Thermostat'
import type { IDataMainListProps } from '../scripts/IData'

const iconMap: Record<string, React.ReactNode> = {
	temperature: <ThermostatIcon />,
	humidity: <WaterDropIcon />,
	atmPressure: <SpeedIcon />,
}

function getLabel(parameter?: string): string {
	if (!parameter) return ''
	return parameter.charAt(0).toUpperCase() + parameter.slice(1)
}

export default function DataMainList({ lastValue }: IDataMainListProps) {
	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	return (
		<>
			<ListItemAvatar>{iconMap[lastValue.parameter || ''] || null}</ListItemAvatar>
			<ListItemText>
				{!isMobile ? (
					<Tooltip title={`Sensor: ID${lastValue.equSensorId ?? ''} Time: ${lastValue.time ?? ''}`}>
						<Typography variant='body2'>
							{`${getLabel(lastValue.parameter)}: ${lastValue.value ?? '-'}${lastValue.unit ?? ''}`}
						</Typography>
					</Tooltip>
				) : (
					<Typography variant='body2'>{`${lastValue.value ?? '-'}${lastValue.unit ?? ''}`}</Typography>
				)}
			</ListItemText>
		</>
	)
}
