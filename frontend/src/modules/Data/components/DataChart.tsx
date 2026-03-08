import { useEffect, useState, useRef, useMemo, lazy, Suspense } from 'react'
import ReactECharts from 'echarts-for-react'
import { Typography, Box, useMediaQuery, useTheme } from '@mui/material'
import LoadingCircle from '../../../components/UI/LoadingCircle'
import type { ISensorData, IEventMarker } from '../scripts/IData'
import { useAppDispatch } from '../../../store/hooks'
import { showAlert } from '../../../store/application-store'
import { useGetDataLogsViewQuery } from '../../../store/api/dataApi'
const DataChartRangeButtons = lazy(() => import('./DataChartRangeButtons'))
const DataChartExportButtons = lazy(() => import('./DataChartExportButtons'))
import formatLocalDateTime from '../../../components/scripts/ComponentsInterface'

export default function DataChart({ equLoggerId, equSensorId }: { equLoggerId: number; equSensorId: number }) {
	const [range, setRange] = useState<string>(() => localStorage.getItem('sensor_range') || '1h')
	const [autoRefreshEnabled, setAutoRefreshEnabled] = useState<boolean>(() => {
		const saved = localStorage.getItem('auto_refresh')
		return saved === null ? true : saved === 'true'
	})
	const [timeLeft, setTimeLeft] = useState<number>(60)
	const refreshInterval = 60

	const [chartData, setChartData] = useState<ISensorData[]>([])
	const chartRef = useRef<ReactECharts>(null)
	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))
	const dispatch = useAppDispatch()

	const query = useMemo(() => {
		const q: any = { $and: [{ equLoggerId }, { equSensorId }] }
		let from: number | undefined
		if (range === '1h') from = Date.now() - 60 * 60 * 1000
		else if (range === '1d') from = Date.now() - 24 * 60 * 60 * 1000
		else if (range === '1w') from = Date.now() - 7 * 24 * 60 * 60 * 1000
		else if (range === '1m') from = Date.now() - 30 * 24 * 60 * 60 * 1000
		else if (range === 'all') from = 0
		if (from !== undefined) {
			const startDate = formatLocalDateTime(from)
			q.$and.push({ time: { $gte: startDate } })
		}
		return q
	}, [equLoggerId, equSensorId, range])

	const { data, isLoading, error, refetch } = useGetDataLogsViewQuery(query, {
		skip: !equLoggerId || !equSensorId,
	})

	useEffect(() => {
		if (!autoRefreshEnabled) return
		const interval = setInterval(() => {
			setTimeLeft(prev => {
				if (prev <= 1) {
					refetch()
					return refreshInterval
				}
				return prev - 1
			})
		}, 1000)
		return () => clearInterval(interval)
	}, [autoRefreshEnabled, refreshInterval, refetch])

	useEffect(() => {
		if (data) {
			let dataChart: ISensorData[] = []
			data.map(d => {
				let dataLog: ISensorData = {
					timestamp: d.time ? d.time : '',
					temperature: 0,
					humidity: 0,
					atmPressure: 0,
					altitude: 0,
					equLoggerId: 0,
					equSensorId: 0,
				}

				if (d.temperature) dataLog.temperature = parseFloat(d.temperature)
				if (d.humidity) dataLog.humidity = parseFloat(d.humidity)
				if (d.atmPressure) dataLog.atmPressure = parseFloat(d.atmPressure)
				if (d.altitude) dataLog.altitude = parseFloat(d.altitude)
				if (d.equSensorId) dataLog.equSensorId = d.equSensorId
				if (d.equLoggerId) dataLog.equLoggerId = d.equLoggerId

				dataChart.push(dataLog)
			})
			setChartData(dataChart)
		}
		if (error) {
			const message = (error as any)?.data?.message || (error as any)?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [data, error, dispatch])

	const rangeLabels: Record<string, string> = {
		'1h': 'Last hour',
		'1d': 'Last day',
		'1w': 'Last week',
		'1m': 'Last month',
		all: 'All data',
	}

	function handleRangeChange(newRange: string): void {
		setRange(newRange)
		localStorage.setItem('sensor_range', newRange)
	}
	function handleReset(): void {
		setRange('1h')
		localStorage.removeItem('sensor_range')
	}
	function refreshData(): void {
		refetch()
		setTimeLeft(refreshInterval)
	}

	const times = chartData.map(d => formatLocalDateTime(d.timestamp))
	const tempData = chartData.map(d => [d.timestamp, d.temperature])
	const humData = chartData.map(d => [d.timestamp, d.humidity])
	const atmPressureData = chartData.map(d => [d.timestamp, d.atmPressure])

	const avgTemp = tempData.reduce((acc: any, d: any) => acc + d[1], 0) / (tempData.length || 1)
	const avgHum = humData.reduce((acc: any, d: any) => acc + d[1], 0) / (humData.length || 1)
	const avgAtmPressure = atmPressureData.reduce((acc: any, d: any) => acc + d[1], 0) / (atmPressureData.length || 1)

	const tempThreshold = 30
	const humThreshold = 30

	const events: IEventMarker[] = chartData
		.filter(d => d.event)
		.map(d => ({
			name: d.event!,
			xAxis: formatLocalDateTime(d.timestamp),
			label: { formatter: d.event! },
		}))

	const timestamps = chartData.map(d => new Date(d.timestamp).getTime()).filter(t => !isNaN(t))
	const minTimestamp = timestamps.length > 0 ? Math.min(...timestamps) : null
	const maxTimestamp = timestamps.length > 0 ? Math.max(...timestamps) : null
	const dateRangeText =
		minTimestamp && maxTimestamp ? `${formatLocalDateTime(minTimestamp)} – ${formatLocalDateTime(maxTimestamp)}` : ''

	if (!isLoading && (!chartData || chartData.length === 0)) {
		return (
			<>
				<Suspense fallback={<LoadingCircle />}>
					<DataChartRangeButtons range={range} handleRangeChange={handleRangeChange} handleReset={handleReset} />
					<DataChartExportButtons
						exportChartImage={exportChartImage}
						range={range}
						chartData={chartData}
						refreshData={refreshData}
						loading={isLoading}
						autoRefreshEnabled={autoRefreshEnabled}
						setAutoRefreshEnabled={setAutoRefreshEnabled}
					/>
				</Suspense>
				<Box display='flex' justifyContent='center' mt={5}>
					<Typography variant='body2' color='text.secondary'>
						No data to display for the selected range.
					</Typography>
				</Box>
			</>
		)
	}

	const option = {
		title: { text: `Sensor data – ${rangeLabels[range]}`, left: 'center' },
		tooltip: { trigger: 'axis', axisPointer: { type: 'cross' } },
		legend: { data: ['Temperature', 'Humidity', 'Pressure'], top: isMobile ? 38 : 30 },
		dataZoom: [
			{ type: 'inside', start: 90, end: 100 },
			{ type: 'slider', start: 90, end: 100 },
		],
		xAxis: [
			{
				type: 'time',
				data: times,
				axisLabel: { rotate: isMobile ? 35 : 0 },
			},
		],
		yAxis: [
			{ type: 'value', name: 'Temperature (°C)', position: 'left' },
			{ type: 'value', name: 'Humidity (%)', position: 'right' },
			{ type: 'value', name: 'Pressure (hPa)', position: 'right' },
		],
		series: [
			{
				name: 'Temperature',
				type: 'line',
				data: tempData,
				yAxisIndex: 0,
				smooth: true,
				showSymbol: false,
				markLine: {
					data: [
						{ yAxis: tempThreshold, label: { formatter: 'Alarm (30°C)' } },
						{ yAxis: avgTemp, label: { formatter: 'Average temperature' }, lineStyle: { type: 'dashed' } },
					],
				},
			},
			{
				name: 'Humidity',
				type: 'line',
				data: humData,
				yAxisIndex: 1,
				smooth: true,
				showSymbol: false,
				markLine: {
					data: [
						{ yAxis: humThreshold, label: { formatter: 'Alarm (30%)' } },
						{ yAxis: avgHum, label: { formatter: 'Average humidity' }, lineStyle: { type: 'dashed' } },
					],
				},
			},
			{
				name: 'Pressure',
				type: 'line',
				data: atmPressureData,
				yAxisIndex: 2,
				smooth: true,
				showSymbol: false,
				markLine: {
					data: [{ yAxis: avgAtmPressure, label: { formatter: 'Average pressure' }, lineStyle: { type: 'dashed' } }],
				},
			},
		],
		markPoint: { data: events },
	}

	function exportChartImage() {
		const echartsInstance = chartRef.current?.getEchartsInstance()
		if (!echartsInstance) return
		const imageData = echartsInstance.getDataURL({
			type: 'png',
			pixelRatio: 2,
			backgroundColor: '#fff',
		})
		const link = document.createElement('a')
		link.href = imageData
		link.download = `chart_${range}.png`
		link.click()
	}

	return (
		<>
			<Suspense fallback={<LoadingCircle />}>
				<DataChartRangeButtons range={range} handleRangeChange={handleRangeChange} handleReset={handleReset} />
				<DataChartExportButtons
					exportChartImage={exportChartImage}
					range={range}
					chartData={chartData}
					refreshData={refreshData}
					loading={isLoading}
					autoRefreshEnabled={autoRefreshEnabled}
					setAutoRefreshEnabled={setAutoRefreshEnabled}
				/>
			</Suspense>
			<Typography variant='body2' align='center' color='text.secondary' gutterBottom>
				{dateRangeText}
			</Typography>
			{autoRefreshEnabled && (
				<Typography variant='body2' align='center' color='text.secondary' sx={{ mb: 2 }}>
					Auto-refresh: {timeLeft}s
				</Typography>
			)}
			{isLoading ? (
				<Box display='flex' justifyContent='center' mt={5}>
					<LoadingCircle />
				</Box>
			) : (
				<ReactECharts
					ref={chartRef}
					option={option}
					style={{ height: isMobile ? '50vh' : '60vh', width: '100%' }}
					notMerge={true}
				/>
			)}
		</>
	)
}
