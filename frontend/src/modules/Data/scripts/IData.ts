import type { EquipmentClass } from '../../Equipment/scripts/EquipmentClass'
import type { DataDefinitionClass } from './DataDefinitionClass'
import type { DataLastValueViewClass } from './DataLastValueViewClass'
import type { GridFilterModel, GridSortModel } from '@mui/x-data-grid'

export interface IDataMainProps {
	equipment: EquipmentClass
	lastValues: DataLastValueViewClass[]
}

export interface IDataMainListProps {
	lastValue: DataLastValueViewClass
}

export interface IDataChartRangeButtonsProps {
	range: string
	handleRangeChange: (range: string) => void
	handleReset: () => void
}

export interface IDataChartExportButtonsProps {
	chartData: ISensorData[]
	exportChartImage: () => void
	range: string
	refreshData: () => void
	loading: boolean
	setAutoRefreshEnabled: (enabled: boolean) => void
	autoRefreshEnabled: boolean
}

export interface ISensorData {
	timestamp: string
	temperature: number
	humidity: number
	atmPressure?: number
	altitude?: number
	equSensorId: number
	equLoggerId: number
	event?: string
}

export interface IEventMarker {
	name: string
	xAxis: string
	label: { formatter: string }
}

export interface IAddDataDefinition {
	id?: number
	name: string | undefined
	unit: string | undefined
	description: string | undefined
}

export interface IDataDefinitionTableProps {
	dataDefinitions: DataDefinitionClass[]
	initSort: GridSortModel
	initFilter: GridFilterModel
}

export interface IAddDataDefinitionProps {
	edit: boolean
	selectedItems?: DataDefinitionClass[]
	handleCloseAdd: () => void
	openAddDialog: boolean
	addItemHandler: (item: IAddDataDefinition[] | IAddDataDefinition) => void
}
