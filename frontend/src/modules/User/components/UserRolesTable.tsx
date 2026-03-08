import { useState, useEffect, useCallback } from 'react'
import { Box, Typography } from '@mui/material'
import { DataGrid, type GridColDef, type GridFilterModel, type GridSortModel } from '@mui/x-data-grid'
import GroupIcon from '@mui/icons-material/Group'
import type { IUserRolesProps } from '../scripts/IUser'

export default function UserRolesTable({ rolesData, initSort, initFilter }: IUserRolesProps) {
	const [sortModel, setSortModel] = useState<GridSortModel>(initSort)
	const [filterModel, setFilterModel] = useState<GridFilterModel>(initFilter)

	useEffect(() => {
		localStorage.setItem('userRolesTableSortModel', JSON.stringify(sortModel))
	}, [sortModel])

	useEffect(() => {
		localStorage.setItem('userRolesTableFilterModel', JSON.stringify(filterModel))
	}, [filterModel])

	const handleSortModelChange = useCallback((newSortModel: GridSortModel) => {
		setSortModel(newSortModel)
	}, [])

	const columns: GridColDef[] = [
		{
			field: 'name',
			headerName: 'Name',
			width: 300,
			valueGetter: (_, row) => `${row.role && row.role.name ? row.role.name : '-'}`,
		},
		{
			field: 'description',
			headerName: 'Description',
			width: 300,
			valueGetter: (_, row) => `${row.role && row.role.description ? row.role.description : '-'}`,
		},
	]

	return (
		<Box sx={{ textAlign: 'center' }}>
			<Box sx={{ textAlign: 'left' }}>
				<Box sx={{ display: 'flex', alignItems: 'center' }}>
					<GroupIcon sx={{ mr: 1 }} />
					<Typography variant='h6' component='p'>
						User roles
					</Typography>
				</Box>
				<Typography component='span'>Database containing all the roles.</Typography>
			</Box>
			<Box sx={{ mt: 4 }}>
				<DataGrid
					getRowId={row => row.roleId}
					rows={rolesData}
					columns={columns}
					initialState={{ pagination: { paginationModel: { page: 0, pageSize: 15 } } }}
					pageSizeOptions={[15, 30, 45]}
					disableRowSelectionOnClick
					checkboxSelection={false}
					sx={{
						border: 'none',
						borderRadius: 3,
						'& .MuiDataGrid-columnHeaders': { backgroundColor: 'rgba(74, 158, 158, 0.06)', fontWeight: 600 },
						'& .MuiDataGrid-row:hover': { backgroundColor: 'rgba(74, 158, 158, 0.04)' },
						'& .MuiDataGrid-cell': { borderColor: 'rgba(0, 0, 0, 0.04)' },
					}}
					density='compact'
					disableColumnResize
					disableColumnSelector
					disableVirtualization
					hideFooterSelectedRowCount
					sortModel={sortModel}
					showToolbar
					onSortModelChange={handleSortModelChange}
					filterModel={filterModel}
					onFilterModelChange={newFilterModel => setFilterModel(newFilterModel)}
				/>
			</Box>
		</Box>
	)
}
