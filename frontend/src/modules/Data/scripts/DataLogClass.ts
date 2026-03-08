import { DataDefinitionClass } from './DataDefinitionClass'
import { EquipmentClass } from '../../Equipment/scripts/EquipmentClass'

export interface DataLogInput {
	id?: number | undefined
	value?: string | undefined
	time?: string | undefined
	dataDefinitionId?: number | undefined
	equLoggerId?: number | undefined
	equSensorId?: number | undefined
	definition?: DataDefinitionClass | undefined
	logger?: EquipmentClass | undefined
	sensor?: EquipmentClass | undefined
	event?: string
}

export class DataLogClass implements DataLogInput {
	id: number | undefined
	value: string | undefined
	time: string | undefined
	dataDefinitionId: number | undefined
	equLoggerId: number | undefined
	equSensorId: number | undefined
	definition: DataDefinitionClass | undefined
	logger: EquipmentClass | undefined
	sensor: EquipmentClass | undefined
	event?: string

	constructor(model: DataLogInput = {}) {
		if (model) {
			Object.assign(this, model)
			if (model.definition) {
				this.definition = new DataDefinitionClass(model.definition)
			}
			if (model.logger) {
				this.logger = new EquipmentClass(model.logger)
			}
			if (model.sensor) {
				this.sensor = new EquipmentClass(model.sensor)
			}
		}
	}
}
