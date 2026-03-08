import type { HouseClass } from './HouseClass'
import type { HouseFloorClass } from './HouseFloorClass'
import type { HouseLoggerClass } from './HouseLoggerClass'
import type { GridFilterModel, GridSortModel } from '@mui/x-data-grid'

export interface IAddHouseProps {
	edit: boolean
	selectedItems?: HouseClass[]
	handleCloseAdd: () => void
	openAddDialog: boolean
	addItemHandler: (item: IAddHouseData[] | IAddHouseData) => void
}

export interface IAddHouseFoorProps {
	edit: boolean
	isDashboard?: boolean
	dashboardData?: HouseClass
	selectedItems?: HouseFloorClass[]
	handleCloseAdd: () => void
	openAddDialog: boolean
	addItemHandler: (item: IAddHouseFloorData[] | IAddHouseFloorData) => void
}

export interface IAddHouseLoggerProps {
	edit: boolean
	selectedItems?: HouseLoggerClass[]
	handleCloseAdd: () => void
	openAddDialog: boolean
	addItemHandler: (item: IAddHouseLoggerData[] | IAddHouseLoggerData) => void
}

export interface IAddHouseData {
	id?: number
	name: string | undefined
	postalCode: string | undefined
	city: string | undefined
	street: string | undefined
	houseNumber: string | undefined
	pictureLink: string | Blob | undefined
}

export interface IAddHouseFloorData {
	id?: number
	name: string | undefined
	layout: string | File | undefined
	houseId: number | undefined
}

export interface IAddHouseLoggerData {
	id?: number
	equLoggerId: number | undefined
	houseFloorId: number | undefined
	posX?: number | undefined
	posY?: number | undefined
}

export interface IAddHouseSensorData {
	id?: number
	equSensorId: number | undefined
}

export interface IAddHouseSensorFunctionData {
	id?: number
	name: string | undefined
}

export interface IHouseTableProps {
	houses: HouseClass[]
	initSort: GridSortModel
	initFilter: GridFilterModel
}

export interface IHouseFloorTableProps {
	houseFloors: HouseFloorClass[]
	initSort: GridSortModel
	initFilter: GridFilterModel
}

export interface IHouseLoggerTableProps {
	houseLoggers: HouseLoggerClass[]
	initSort: GridSortModel
	initFilter: GridFilterModel
}
