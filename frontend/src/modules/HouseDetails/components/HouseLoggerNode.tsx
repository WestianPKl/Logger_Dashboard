import { useEffect, useMemo, useState } from 'react'
import { Card, Typography } from '@mui/material'
import { useReactFlow } from '@xyflow/react'
import { useGetDataLastValuesViewQuery, useGetDataConnectedSensorViewQuery } from '../../../store/api/dataApi'
import HouseDetailsLoggerNodeDialog from './HouseDetailsLoggerNodeDialog'
import HouseDetailsLoggerNewNodeDialog from './HouseDetailsLoggerNewNodeDialog'
import { useAddHouseLoggerMutation, useDeleteHouseLoggerMutation } from '../../../store/api/houseApi'
import { useAppDispatch } from '../../../store/hooks'
import { showAlert } from '../../../store/application-store'
import LoadingCircle from '../../../components/UI/LoadingCircle'
import type { IAddHouseLoggerData } from '../../House/scripts/IHouse'
import type { IHouseLoggerData, IHouseLoggerNode } from '../scripts/IHouseDetails'
import { socket } from '../../../socket/socket'
import { skipToken } from '@reduxjs/toolkit/query'
import { useRevalidator } from 'react-router'
import type { DataLastValueViewClass } from '../../Data/scripts/DataLastValueViewClass'

type Props = {
	id: string
	data: IHouseLoggerNode
	positionAbsoluteX: number
	positionAbsoluteY: number
}

const HouseDetailsLoggerNode = ({ id, data, positionAbsoluteX, positionAbsoluteY }: Props) => {
	const [detailsDialog, setDetailsDialog] = useState(false)
	const [lastValue, setLastValue] = useState<DataLastValueViewClass[]>([])
	const [statusColors, setStatusColors] = useState<'success.main' | 'error.main'>('success.main')

	const [editMode, setEditMode] = useState<boolean>(data.editMode)
	const [equLoggerId, setEquLoggerId] = useState<number>(data.equLoggerId)

	const [loggerData, setLoggerData] = useState<IHouseLoggerData>(() =>
		data.editMode
			? {
					floorId: data.floorId,
					id: data.equLoggerId,
					serialNumber: data.label,
					equVendor: data.equVendor,
					equModel: data.equModel,
					houseLoggerId: data.houseLoggerId,
				}
			: {
					floorId: data.floorId,
					id: data.equLoggerId,
				},
	)

	const queryArgs = useMemo(() => {
		const houseLoggerId = loggerData.houseLoggerId ?? data.houseLoggerId
		const houseFloorId = loggerData.floorId ?? data.floorId
		if (!houseLoggerId || !houseFloorId) return skipToken
		return { houseLoggerId, houseFloorId }
	}, [loggerData.houseLoggerId, loggerData.floorId, data.houseLoggerId, data.floorId])

	const {
		data: lastValueData = [],
		isLoading: lastValueLoading,
		error: lastValueError,
		refetch: refetchLastValue,
		isUninitialized: lastValueUninitialized,
	} = useGetDataLastValuesViewQuery(queryArgs)

	const {
		data: connectedSensors = [],
		isLoading: connectedSensorsLoading,
		error: connectedSensorsError,
		refetch: refetchConnectedSensors,
		isUninitialized: connectedSensorsUninitialized,
	} = useGetDataConnectedSensorViewQuery(queryArgs)

	const [addHouseLogger] = useAddHouseLoggerMutation()
	const [deleteHouseLogger] = useDeleteHouseLoggerMutation()

	const dispatch = useAppDispatch()
	const revalidator = useRevalidator()
	const { setNodes, getNode } = useReactFlow()

	useEffect(() => {
		setLastValue(lastValueData)

		const statuses = lastValueData.map(e => {
			if (!e.time) return false
			const ageMs = Date.now() - new Date(e.time).getTime()
			return ageMs < 30 * 60 * 1000
		})

		const ok = statuses.length > 0 && statuses.every(Boolean)
		setStatusColors(ok ? 'success.main' : 'error.main')
	}, [lastValueData])

	useEffect(() => {
		if (!equLoggerId) return

		const event = `loggerData_${equLoggerId}`
		const onRefreshDataEvent = () => {
			if (!lastValueUninitialized) refetchLastValue()
			if (!connectedSensorsUninitialized) refetchConnectedSensors()
		}

		socket.on(event, onRefreshDataEvent)
		return () => {
			socket.off(event, onRefreshDataEvent)
		}
	}, [equLoggerId, refetchLastValue, refetchConnectedSensors, lastValueUninitialized, connectedSensorsUninitialized])

	useEffect(() => {
		setLoggerData(prev =>
			editMode
				? {
						floorId: data.floorId,
						id: equLoggerId,
						serialNumber: prev.serialNumber ?? data.label,
						equVendor: prev.equVendor ?? data.equVendor,
						equModel: prev.equModel ?? data.equModel,
						houseLoggerId: prev.houseLoggerId ?? data.houseLoggerId,
					}
				: { floorId: data.floorId, id: equLoggerId },
		)
	}, [editMode, data.floorId, data.label, data.equVendor, data.equModel, data.houseLoggerId, equLoggerId])

	useEffect(() => {
		const err = lastValueError || connectedSensorsError
		if (err) {
			const message = (err as any)?.data?.message || (err as any)?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [lastValueError, connectedSensorsError, dispatch])

	async function addItemHandler(item: IAddHouseLoggerData | IAddHouseLoggerData[]): Promise<void> {
		try {
			setDetailsDialog(false)

			if (Array.isArray(item)) return

			let x = positionAbsoluteX
			let y = positionAbsoluteY

			if (data.id) {
				const node = getNode(data.id)
				if (node?.position?.x != null) x = node.position.x
				if (node?.position?.y != null) y = node.position.y
			}

			item.posX = x
			item.posY = y

			const logger = await addHouseLogger(item).unwrap()

			setLoggerData({
				floorId: logger.houseFloorId,
				id: logger.equLoggerId,
				houseLoggerId: logger.id,
				serialNumber: logger.logger?.serialNumber,
				equModel: logger.logger?.model?.name,
				equVendor: logger.logger?.vendor?.name,
			})

			if (logger.equLoggerId) setEquLoggerId(logger.equLoggerId)
			setEditMode(true)

			if (!lastValueUninitialized) refetchLastValue()
			if (!connectedSensorsUninitialized) refetchConnectedSensors()

			dispatch(showAlert({ message: 'New house logger added', severity: 'success' }))

			revalidator.revalidate()
		} catch (err: any) {
			const message = err?.data?.message || err?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}

	function onDoubleClickHandler(e: any): void {
		if (e.detail > 1) setDetailsDialog(true)
	}

	async function handleClickDeleteNode(nodeData: any): Promise<void> {
		try {
			if (nodeData.houseLoggerId) {
				await deleteHouseLogger({ id: nodeData.houseLoggerId }).unwrap()
				setNodes(nodes => nodes.filter(n => n.id !== id))
				dispatch(showAlert({ message: 'Logger node deleted', severity: 'success' }))
				revalidator.revalidate()
			} else {
				setNodes(nodes => nodes.filter(n => n.id !== id))
			}
		} catch (err: any) {
			const message = err?.data?.message || err?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}

	if (lastValueLoading || connectedSensorsLoading) return <LoadingCircle />

	return (
		<Card
			sx={{
				display: 'flex',
				justifyContent: 'center',
				alignItems: 'center',
				width: 60,
				height: 60,
				backgroundColor: statusColors,
			}}
			onClick={onDoubleClickHandler}>
			<Typography variant='body2'>
				{editMode
					? `ID${equLoggerId}`.length > 7
						? `ID${equLoggerId}`.substring(0, 6) + '...'
						: `ID${equLoggerId}`
					: `ID-N`}
			</Typography>

			{editMode ? (
				<HouseDetailsLoggerNodeDialog
					editModeProps={data.editModeProps}
					loggerData={loggerData}
					connectedSensors={connectedSensors}
					lastValueData={lastValue}
					onCloseDialog={() => setDetailsDialog(false)}
					detailsDialog={detailsDialog}
					handleClickDeleteNode={handleClickDeleteNode}
				/>
			) : (
				<HouseDetailsLoggerNewNodeDialog
					loggerData={loggerData}
					addItemHandler={addItemHandler}
					onCloseDialog={() => setDetailsDialog(false)}
					detailsDialog={detailsDialog}
					handleClickDeleteNode={handleClickDeleteNode}
				/>
			)}
		</Card>
	)
}

export default HouseDetailsLoggerNode
