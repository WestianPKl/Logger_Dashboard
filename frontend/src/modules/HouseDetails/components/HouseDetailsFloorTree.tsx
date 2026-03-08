import { useCallback, useEffect, useState } from 'react'
import {
	ReactFlow,
	Controls,
	applyNodeChanges,
	useNodesState,
	Background,
	MiniMap,
	useReactFlow,
	Panel,
	type Node,
} from '@xyflow/react'
import '@xyflow/react/dist/base.css'
import type { IHouseDetailsFloorTreeProps } from '../scripts/IHouseDetails'
import { useUpdateHouseFloorLayoutMutation } from '../../../store/api/houseApi'
import { showAlert } from '../../../store/application-store'
import { useAppDispatch } from '../../../store/hooks'
import FloorNode from './HouseNode'
import LoggerNode from './HouseLoggerNode'
import { Button } from '@mui/material'
import { useRevalidator } from 'react-router'

const getNodeId = () => `randomnode_${+new Date()}`

export default function HouseDetailsFloorTree({
	floorId,
	floor,
	floorViewport,
	editMode,
}: IHouseDetailsFloorTreeProps) {
	const [updateHouseFloorLayout] = useUpdateHouseFloorLayoutMutation()
	const [nodes, setNodes] = useNodesState<any>([])
	const [rfInstance, setRfInstance] = useState<any>(null)
	const { setViewport } = useReactFlow()
	const [isSaved, setIsSaved] = useState<boolean>(false)
	const revalidator = useRevalidator()

	const dispatch = useAppDispatch()

	useEffect(() => {
		setNodes([
			{
				id: `Floor_${floor.id}`,
				position: { x: floor.posX, y: floor.posY },
				dragHandle: '',
				data: { background: `${import.meta.env.VITE_API_IP}/${floor.layout}?w=500&h=500&format=webp` },
				style: { backgroundImage: `${import.meta.env.VITE_API_IP}/${floor.layout}?w=500&h=500&format=webp` },
				type: 'floorNode',
			},
		])
		let testNodes: any = []
		floor.loggers.map(item => {
			testNodes.push({
				id: `Logger_${item.id}`,
				type: 'loggerNode',
				position: { x: item.posX, y: item.posY },
				data: {
					label: item.logger?.serialNumber,
					houseLoggerId: item.id,
					equLoggerId: item.equLoggerId,
					equModel: item.logger?.model?.name,
					equVendor: item.logger?.vendor?.name,
					floorId: floor.id,
					editMode: true,
					editModeProps: editMode,
				},
				parentId: `Floor_${floor.id}`,
				extent: 'parent',
			})
		})
		setNodes(nds => nds.concat(testNodes))
		setViewport(floorViewport)
	}, [setViewport, floorViewport])

	const onNodesChange = useCallback(
		(changes: any) => setNodes((nds: any) => applyNodeChanges(changes, nds)),
		[setNodes],
	)

	const onAdd = useCallback(() => {
		const newNodeId = getNodeId()
		const newNode: Node = {
			id: newNodeId,
			position: {
				x: floorViewport.x,
				y: floorViewport.y,
			},
			type: 'loggerNode',
			data: {
				id: newNodeId,
				label: 'New logger',
				editMode: false,
				floorId: floorId,
			},
			parentId: `Floor_${floorId}`,
			extent: 'parent',
		}
		setNodes(nds => nds.concat(newNode))
	}, [setNodes])

	const onSave = useCallback(async () => {
		if (rfInstance) {
			const flow = rfInstance.toObject()
			if (floorId) {
				let testNodes: { id: any; posX: number; posY: number }[] = []
				let floorPosX: number = 0
				let floorPosY: number = 0
				localStorage.setItem(`Floor_${floorId}`, JSON.stringify(flow))
				setIsSaved(true)
				flow.nodes.forEach((e: Node) => {
					if (e.type === 'loggerNode') {
						testNodes.push({ id: e.data.houseLoggerId, posX: e.position.x, posY: e.position.y })
					}
				})
				flow.nodes.forEach((e: Node) => {
					if (e.type === 'floorNode') {
						floorPosX = e.position.x
						floorPosY = e.position.y
					}
				})
				const data = {
					x: flow.viewport.x,
					y: flow.viewport.y,
					posX: floorPosX,
					posY: floorPosY,
					zoom: flow.viewport.zoom,

					loggers: testNodes,
				}
				try {
					await updateHouseFloorLayout({ body: data, id: floorId })
					dispatch(showAlert({ message: 'Layout saved', severity: 'success' }))
					revalidator.revalidate()
				} catch (err: any) {
					const message = err?.data?.message || err?.message || 'Something went wrong'
					dispatch(showAlert({ message, severity: 'error' }))
				}
			}
		}
	}, [rfInstance])

	const onRestore = useCallback(() => {
		const restoreFlow = async () => {
			const flowData = localStorage.getItem(`Floor_${floorId}`)
			if (flowData) {
				const flow = JSON.parse(flowData)
				if (flow) {
					const { x = 0, y = 0, zoom = 1 } = flow.viewport
					setNodes(flow.nodes || [])
					setViewport({ x, y, zoom })
				}
			}
		}
		restoreFlow()
	}, [setNodes, setViewport])

	return (
		<div style={{ height: '100%' }}>
			<ReactFlow
				nodes={nodes}
				nodeTypes={{
					floorNode: FloorNode,
					loggerNode: LoggerNode,
				}}
				onNodesChange={onNodesChange}
				onInit={setRfInstance}
				elementsSelectable={true}
				nodesConnectable={editMode ? true : false}
				nodesDraggable={editMode ? true : false}
				zoomOnScroll={editMode ? true : false}
				zoomOnDoubleClick={editMode ? true : false}
				panOnDrag={editMode ? true : false}>
				{editMode && (
					<>
						<Background />
						<MiniMap />
						<Controls />
						<Panel position='top-right'>
							<Button sx={{ marginRight: '0.2rem' }} variant='contained' size='small' onClick={onSave}>
								save
							</Button>
							<Button
								disabled={!isSaved}
								sx={{ marginRight: '0.2rem' }}
								variant='contained'
								size='small'
								onClick={onRestore}>
								restore
							</Button>
							<Button variant='contained' size='small' onClick={onAdd}>
								add logger
							</Button>
						</Panel>
					</>
				)}
			</ReactFlow>
		</div>
	)
}
