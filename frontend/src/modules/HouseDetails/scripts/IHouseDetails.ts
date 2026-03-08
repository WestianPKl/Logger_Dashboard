import type { DataConnectedSensorViewClass } from '../../Data/scripts/DataConnectedSensorViewClass'
import type { DataLastValueViewClass } from '../../Data/scripts/DataLastValueViewClass'
import type { HouseFloorClass } from '../../House/scripts/HouseFloorClass'
import type { EquipmentClass } from '../../Equipment/scripts/EquipmentClass'
import type { HouseClass } from '../../House/scripts/HouseClass'
import type { IAddHouseData, IAddHouseFloorData, IAddHouseLoggerData } from '../../House/scripts/IHouse'

export interface IHouseDetailsFloorProps {
	floor: HouseFloorClass
	houseId?: number | undefined
}

export interface IHouseNode {
	id: number | undefined
	position: { x: number | undefined; y: number | undefined }
	data: { label: string | undefined }
}

export interface IHouseDetailsFloorTreeProps {
	floorId: number | undefined
	floorViewport: { x: number; y: number; zoom: number }
	editMode: boolean
	floor: HouseFloorClass
}

export interface IHouseDetailsLoggerNodeDialogProps {
	loggerData: IHouseLoggerData
	lastValueData: DataLastValueViewClass[]
	connectedSensors: DataConnectedSensorViewClass[]
	detailsDialog: boolean
	onCloseDialog: () => void
	handleClickDeleteNode: (data: any) => void
	editModeProps: boolean
}

export interface IHouseDetailsNewLoggerNodeDialogProps {
	loggerData: IHouseLoggerData
	detailsDialog: boolean
	onCloseDialog: () => void
	addItemHandler: (item: IAddHouseLoggerData[] | IAddHouseLoggerData) => void
	handleClickDeleteNode: (data: any) => void
}

export interface IHouseDetailsLoggerNodeListProps {
	lastValue: DataLastValueViewClass
}

export interface IHouseDetailsMobileCardProps {
	logger: EquipmentClass
	floorId: number | undefined
	houseLoggerId: number | undefined
}

export interface IHouseEditViewProps {
	data: HouseClass
}

export interface IHouseLoggerNode {
	id?: string
	label: string
	houseLoggerId: number
	equLoggerId: number
	equModel: string
	equVendor: string
	floorId: number
	editMode: boolean
	editModeProps: boolean
}

export interface IHouseLoggerData {
	floorId: number | undefined
	id: number | undefined
	serialNumber?: string
	equVendor?: string
	equModel?: string
	houseLoggerId?: number
}

export interface IHouseEditFormProps {
	house: HouseClass
	editHouseHandler: (item: IAddHouseData[] | IAddHouseData) => Promise<void>
	addHouseFloorHandler: (item: IAddHouseFloorData[] | IAddHouseFloorData) => Promise<void>
}
