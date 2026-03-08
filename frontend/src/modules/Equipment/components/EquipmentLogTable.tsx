import { useMemo } from 'react'
import { Box, Typography, Icon } from '@mui/material'
import { DataGrid, type GridColDef } from '@mui/x-data-grid'
import type { IEquipmentLogTableProps } from '../scripts/IEquipment'
import MergeTypeIcon from '@mui/icons-material/MergeType'
import formatLocalDateTime from '../../../components/scripts/ComponentsInterface'

export default function EquipmentLogTable({ equipment }: IEquipmentLogTableProps) {
	const formatDate = (dateString: string | null | undefined) => {
		if (!dateString) return 'N/A'
		return formatLocalDateTime(dateString)
	}

	return (
		<>
			<Box sx={{ mb: 3 }}>
				<Typography variant='h6' gutterBottom>
					Equipment ID: {equipment.id} - Serial Number: {equipment.serialNumber}
				</Typography>
				<Typography variant='subtitle2' color='text.secondary' gutterBottom>
					Last seen: {formatDate(equipment.stats?.lastSeen)}
				</Typography>

				<Box sx={{ display: 'grid', gridTemplateColumns: { xs: '1fr', md: '1fr 1fr' }, gap: 2, mt: 2 }}>
					<Box sx={{ p: 2, bgcolor: 'background.paper', borderRadius: 1, border: '1px solid', borderColor: 'divider' }}>
						<Typography variant='subtitle1' fontWeight='bold' gutterBottom>
							Controller Module
						</Typography>
						<Typography variant='body2'>S/N: {equipment.stats?.snContr ?? 'N/A'}</Typography>
						<Typography variant='body2'>Firmware: {equipment.stats?.fwContr ?? 'N/A'}</Typography>
						<Typography variant='body2'>HW version: {equipment.stats?.hwContr ?? 'N/A'}</Typography>
						<Typography variant='body2'>Build date: {equipment.stats?.buildContr ?? 'N/A'}</Typography>
						<Typography variant='body2'>Production date: {equipment.stats?.prodContr ?? 'N/A'}</Typography>
					</Box>
					<Box sx={{ p: 2, bgcolor: 'background.paper', borderRadius: 1, border: '1px solid', borderColor: 'divider' }}>
						<Typography variant='subtitle1' fontWeight='bold' gutterBottom>
							Communication Module
						</Typography>
						<Typography variant='body2'>IP address: {equipment.stats?.ipAddress ?? 'N/A'}</Typography>
						<Typography variant='body2'>S/N: {equipment.stats?.snCom ?? 'N/A'}</Typography>
						<Typography variant='body2'>Firmware: {equipment.stats?.fwCom ?? 'N/A'}</Typography>
						<Typography variant='body2'>HW version: {equipment.stats?.hwCom ?? 'N/A'}</Typography>
						<Typography variant='body2'>Build date: {equipment.stats?.buildCom ?? 'N/A'}</Typography>
						<Typography variant='body2'>Production date: {equipment.stats?.prodCom ?? 'N/A'}</Typography>
					</Box>
				</Box>
			</Box>
			<Box sx={{ textAlign: 'center' }}>
				<Box sx={{ textAlign: 'left' }}>
					<Box sx={{ display: 'flex' }}>
						<Icon sx={{ mr: '0.5rem' }}>
							<MergeTypeIcon />
						</Icon>
						<Typography variant='h6' component='p'>
							Equipment Logs table
						</Typography>
					</Box>
					<Typography component='span'>
						Your database containg all the equipment logs that have been registered.
					</Typography>
				</Box>
				<Box sx={{ mt: '2rem' }}>
					<Box sx={{ mb: '1rem', textAlign: 'right' }}></Box>
					<DataGrid
						rows={equipment.logs}
						columns={useMemo<GridColDef[]>(
							() => [
								{ field: 'id', headerName: 'ID', width: 100 },
								{
									field: 'message',
									headerName: 'Message',
									width: 1000,
									valueGetter: (_: any, row: any) => {
										try {
											const raw = row.message ?? ''
											const jsonText = raw
												.replace(/\{\/\*\s*/g, '')
												.replace(/\s*\*\/\}/g, '')
												.replace(/^Status update:\s*/i, '')
												.trim()

											const obj = JSON.parse(jsonText) as Record<string, unknown>

											return Object.entries(obj)
												.map(([k, v]) => `${k}: ${typeof v === 'object' ? JSON.stringify(v) : String(v)}`)
												.join(', ')
										} catch {
											return ''
										}
									},
								},
								{ field: 'type', headerName: 'Type', width: 150 },
								{
									field: 'createdAt',
									headerName: 'Created At',
									width: 200,
									valueGetter: (_, row) => `${formatLocalDateTime(row.createdAt)}`,
								},
							],
							[],
						)}
						initialState={{ pagination: { paginationModel: { page: 0, pageSize: 15 } } }}
						pageSizeOptions={[15, 30, 45]}
						checkboxSelection={false}
						disableRowSelectionOnClick={true}
						sx={{
							border: 'none',
							borderRadius: 3,
							width: '100%',
							'& .MuiDataGrid-columnHeaders': { backgroundColor: 'rgba(74, 158, 158, 0.06)', fontWeight: 600 },
							'& .MuiDataGrid-row:hover': { backgroundColor: 'rgba(74, 158, 158, 0.04)' },
							'& .MuiDataGrid-cell': { borderColor: 'rgba(0, 0, 0, 0.04)' },
						}}
						density='comfortable'
						disableColumnResize={true}
						disableColumnSelector={true}
						disableMultipleRowSelection={true}
						showToolbar
					/>
				</Box>
			</Box>
			<Box sx={{ textAlign: 'center' }}>
				<Box sx={{ textAlign: 'left' }}>
					<Box sx={{ display: 'flex' }}>
						<Icon sx={{ mr: '0.5rem' }}>
							<MergeTypeIcon />
						</Icon>
						<Typography variant='h6' component='p'>
							Equipment Errors table
						</Typography>
					</Box>
					<Typography component='span'>
						Your database containg all the equipment errors that have been registered.
					</Typography>
				</Box>
				<Box sx={{ mt: '2rem' }}>
					<Box sx={{ mb: '1rem', textAlign: 'right' }}></Box>
					<DataGrid
						rows={equipment.errors}
						columns={useMemo<GridColDef[]>(
							() => [
								{ field: 'id', headerName: 'ID', width: 100 },
								{ field: 'message', headerName: 'Message', width: 400 },
								{ field: 'details', headerName: 'Details', width: 500 },
								{ field: 'type', headerName: 'Type', width: 150 },
								{ field: 'severity', headerName: 'Severity', width: 150 },
								{
									field: 'createdAt',
									headerName: 'Creation date',
									width: 160,
									valueGetter: (_, row) => `${formatLocalDateTime(row.createdAt)}`,
								},
							],
							[],
						)}
						initialState={{ pagination: { paginationModel: { page: 0, pageSize: 15 } } }}
						pageSizeOptions={[15, 30, 45]}
						checkboxSelection={false}
						disableRowSelectionOnClick={true}
						sx={{
							border: 'none',
							borderRadius: 3,
							width: '100%',
							'& .MuiDataGrid-columnHeaders': { backgroundColor: 'rgba(74, 158, 158, 0.06)', fontWeight: 600 },
							'& .MuiDataGrid-row:hover': { backgroundColor: 'rgba(74, 158, 158, 0.04)' },
							'& .MuiDataGrid-cell': { borderColor: 'rgba(0, 0, 0, 0.04)' },
						}}
						density='comfortable'
						disableColumnResize={true}
						disableColumnSelector={true}
						disableMultipleRowSelection={true}
						showToolbar
					/>
				</Box>
			</Box>
		</>
	)
}
