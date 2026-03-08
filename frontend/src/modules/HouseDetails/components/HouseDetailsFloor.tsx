import { Container, Button, Box } from '@mui/material'
import { ReactFlowProvider } from '@xyflow/react'
import type { IHouseDetailsFloorProps } from '../scripts/IHouseDetails'
import HouseDetailsFloorTree from './HouseDetailsFloorTree'
import { canWrite } from '../../../store/auth-actions'
import { useAppSelector } from '../../../store/hooks'
import { useState } from 'react'
import EditIcon from '@mui/icons-material/Edit'
import CloseIcon from '@mui/icons-material/Close'
import HouseDetailsEditFloor from './HouseDetailsEditFloor'

export default function HouseDetailsFloor({ floor, houseId }: IHouseDetailsFloorProps) {
	const [editMode, setEditMode] = useState<boolean>(false)
	const isWritable = useAppSelector(state => canWrite('house', 'houseFloor')(state))

	return (
		<Container sx={{ height: 500 }}>
			<Box sx={{ margin: 0, padding: 0, display: 'flex', justifyContent: 'end' }}>
				{editMode && <HouseDetailsEditFloor floor={floor} houseId={houseId} />}
				{isWritable && (
					<Button
						startIcon={editMode ? <CloseIcon /> : <EditIcon />}
						variant='contained'
						onClick={() => {
							setEditMode(prevMode => !prevMode)
						}}>
						{editMode ? 'Leave' : 'Edit'}
					</Button>
				)}
			</Box>
			<ReactFlowProvider>
				<HouseDetailsFloorTree
					floor={floor}
					floorId={floor.id}
					floorViewport={{ x: floor.x, y: floor.y, zoom: floor.zoom }}
					editMode={editMode}
				/>
			</ReactFlowProvider>
		</Container>
	)
}
