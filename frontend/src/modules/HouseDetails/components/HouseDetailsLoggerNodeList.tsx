import { ListItemText, Typography, useMediaQuery, useTheme, ListItemAvatar, Tooltip } from '@mui/material'
import WaterDropIcon from '@mui/icons-material/WaterDrop'
import SpeedIcon from '@mui/icons-material/Speed'
import ThermostatIcon from '@mui/icons-material/Thermostat'
import ElectricMeterIcon from '@mui/icons-material/ElectricMeter'
import type { IHouseDetailsLoggerNodeListProps } from '../scripts/IHouseDetails'
import { useEffect, useState } from 'react'

export default function HouseDetailsLoggerNodeList({ lastValue }: IHouseDetailsLoggerNodeListProps) {
	const [parameter, setParameter] = useState<string>('')

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	if (!lastValue) return null

	let IconComponent = null
	switch (lastValue.parameter) {
		case 'temperature':
			IconComponent = <ThermostatIcon />
			break
		case 'humidity':
			IconComponent = <WaterDropIcon />
			break
		case 'atmPressure':
			IconComponent = <SpeedIcon />
			break
		case 'voltage':
			IconComponent = <ElectricMeterIcon />
			break
		default:
			IconComponent = null
	}

	useEffect(() => {
		let para = lastValue.parameter
		if (para && typeof para === 'string') {
			if (para === 'atmPressure') {
				para = 'Atmospheric Pressure'
			}
			setParameter(para)
		}
	}, [lastValue.parameter])

	return (
		<>
			<ListItemAvatar>{IconComponent}</ListItemAvatar>
			<ListItemText>
				{!isMobile ? (
					<Tooltip title={`Sensor: ID${lastValue.equSensorId} • Time: ${lastValue.time}`}>
						<Typography variant='body2'>
							{parameter && typeof parameter === 'string'
								? `${parameter.charAt(0).toUpperCase()}${parameter.slice(1)}`
								: '-'}
							: {`${lastValue.value ?? '-'}${lastValue.unit ? ` ${lastValue.unit}` : ''}`}
						</Typography>
					</Tooltip>
				) : (
					<Typography variant='body2'>{`${lastValue.value ?? '-'}${
						lastValue.unit ? ` ${lastValue.unit}` : ''
					}`}</Typography>
				)}
			</ListItemText>
		</>
	)
}
