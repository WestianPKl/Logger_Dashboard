import {
	Box,
	Button,
	Dialog,
	useMediaQuery,
	useTheme,
	Card,
	CardContent,
	CardHeader,
	Typography,
	List,
	ListItem,
	IconButton,
} from '@mui/material'
import DeleteIcon from '@mui/icons-material/Delete'
import HouseDetailsLoggerNodeList from './HouseDetailsLoggerNodeList'
import type { IHouseDetailsLoggerNodeDialogProps } from '../scripts/IHouseDetails'
import { canDelete } from '../../../store/auth-actions'
import { useAppSelector } from '../../../store/hooks'
import { useNavigate } from 'react-router'

export default function HouseDetailsLoggerNodeDialog({
	loggerData,
	lastValueData,
	connectedSensors,
	detailsDialog,
	onCloseDialog,
	handleClickDeleteNode,
	editModeProps,
}: IHouseDetailsLoggerNodeDialogProps) {
	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))
	const navigate = useNavigate()

	function onDataClick(): void {
		navigate(`/data/data-logger/${loggerData.id}`)
	}

	function onLogClick(): void {
		navigate(`/logs/${loggerData.id}`)
	}

	const isDeletable = useAppSelector(state => canDelete('house', 'houseFloor')(state))

	return (
		<Dialog open={detailsDialog} onClose={onCloseDialog} closeAfterTransition={false}>
			<Card sx={{ maxWidth: 345 }}>
				{editModeProps && isDeletable && (
					<Box sx={{ m: 0, p: 0, display: 'flex', justifyContent: 'end' }}>
						<IconButton
							sx={{ ml: '0.3rem' }}
							color='error'
							type='button'
							size={isMobile ? 'small' : 'medium'}
							onClick={() => handleClickDeleteNode(loggerData)}>
							<DeleteIcon />
						</IconButton>
					</Box>
				)}

				<CardHeader
					sx={{ textAlign: 'center' }}
					title={<Typography variant='h6'>{`Logger ID${loggerData.id}`}</Typography>}
					subheader={
						<>
							<Typography variant='body1'>{`${loggerData.equVendor} ${loggerData.equModel} S/N ${loggerData.serialNumber}`}</Typography>
							{connectedSensors.length > 0 && (
								<>
									<Typography variant='body2'>Connected sensors:</Typography>
									{connectedSensors.map(e => (
										<Typography
											variant='body2'
											key={
												e.equSensorId
											}>{`ID${e.equSensorId} ${e.sensorVendor} ${e.sensorModel} ${e.sensorSerialNumber}`}</Typography>
									))}
								</>
							)}
						</>
					}
				/>
				<CardContent>
					{lastValueData.length > 0 && <Typography variant='body2'>Last values:</Typography>}
					<Box sx={{ marginTop: 0, marginBottom: '1rem', textAlign: 'center' }}>
						{lastValueData.length > 0 && (
							<List sx={{ margin: 0, padding: 0 }}>
								{lastValueData.map(item => (
									<ListItem sx={{ margin: 0, padding: 0 }} key={item.id}>
										<HouseDetailsLoggerNodeList lastValue={item} />
									</ListItem>
								))}
							</List>
						)}
						{lastValueData.length === 0 && <Typography>No data</Typography>}
					</Box>
					<Box sx={{ margin: 0, padding: 0, display: 'flex', justifyContent: 'end' }}>
						{lastValueData.length > 0 && (
							<Button sx={{ mr: 1 }} variant='outlined' size={isMobile ? 'small' : 'medium'} onClick={onDataClick}>
								Data
							</Button>
						)}
						<Button sx={{ mr: 1 }} variant='outlined' size={isMobile ? 'small' : 'medium'} onClick={onLogClick}>
							Log
						</Button>
						<Button variant='outlined' size={isMobile ? 'small' : 'medium'} onClick={onCloseDialog}>
							Close
						</Button>
					</Box>
				</CardContent>
			</Card>
		</Dialog>
	)
}
