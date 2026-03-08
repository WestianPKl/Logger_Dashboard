import { Button, CircularProgress, FormControlLabel, Stack, Switch, useMediaQuery, useTheme } from '@mui/material'
import type { IDataChartExportButtonsProps } from '../scripts/IData'
import { saveAs } from 'file-saver'
import * as XLSX from 'xlsx'
import formatLocalDateTime from '../../../components/scripts/ComponentsInterface'

export default function DataChartExportButtons({
	exportChartImage,
	chartData,
	refreshData,
	loading,
	autoRefreshEnabled,
	range,
	setAutoRefreshEnabled,
}: IDataChartExportButtonsProps) {
	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	function exportToExcel(): void {
		const wsData = [
			[
				'Datetime',
				'Temperature (°C)',
				'Humidity (%)',
				'Pressure (hPA)',
				'Altitude (m)',
				'LoggerId',
				'SensorId',
				'Events',
			],
		]
		chartData.forEach((d: any) => {
			wsData.push([
				formatLocalDateTime(d.timestamp),
				d.temperature,
				d.humidity,
				d.atmPressure,
				d.altitude,
				d.equLoggerId,
				d.equSensorId,
				d.event || '',
			])
		})
		const worksheet = XLSX.utils.aoa_to_sheet(wsData)
		const workbook = XLSX.utils.book_new()
		XLSX.utils.book_append_sheet(workbook, worksheet, 'Data')
		const excelBuffer = XLSX.write(workbook, { bookType: 'xlsx', type: 'array' })
		const blob = new Blob([excelBuffer], { type: 'application/octet-stream' })
		saveAs(blob, `data_${range}.xlsx`)
	}

	function handleAutoRefreshToggle(): void {
		const newValue = !autoRefreshEnabled
		setAutoRefreshEnabled(newValue)
		localStorage.setItem('auto_refresh', String(newValue))
	}

	return (
		<Stack
			direction={isMobile ? 'column' : 'row'}
			spacing={isMobile ? 0 : 2}
			flexWrap='wrap'
			alignItems='center'
			mb={2}>
			<Button
				sx={{ mb: isMobile ? 1 : 0 }}
				size={isMobile ? 'small' : 'medium'}
				variant='contained'
				onClick={exportChartImage}>
				Export to PNG
			</Button>
			<Button
				sx={{ mb: isMobile ? 1 : 0 }}
				size={isMobile ? 'small' : 'medium'}
				variant='contained'
				onClick={exportToExcel}>
				Export data to Excel
			</Button>
			<Button
				sx={{ mb: isMobile ? 1 : 0 }}
				size={isMobile ? 'small' : 'medium'}
				variant='contained'
				color='success'
				onClick={refreshData}
				disabled={loading}
				startIcon={loading ? <CircularProgress size={18} color='inherit' /> : null}>
				Refresh data
			</Button>
			<FormControlLabel
				control={
					<Switch
						size={isMobile ? 'small' : 'medium'}
						checked={autoRefreshEnabled}
						onChange={handleAutoRefreshToggle}
						color='primary'
					/>
				}
				label='Auto-refresh'
			/>
		</Stack>
	)
}
