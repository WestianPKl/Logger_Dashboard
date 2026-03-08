import { DataLogClass } from './DataLogClass'
import { EquipmentClass } from '../../Equipment/scripts/EquipmentClass'

export interface DataLastValueInput {
	id?: number | undefined
	dataLogId?: number | undefined
	equLoggerId?: number | undefined
	equSensorId?: number | undefined
	dataDefinitionId?: number | undefined
	log?: DataLogClass | undefined
	createdAt?: string | undefined
	updatedAt?: string | undefined
	sensor?: EquipmentClass | undefined
}

export class DataLastValueClass implements DataLastValueInput {
	id: number | undefined
	dataLogId: number | undefined
	equLoggerId: number | undefined
	equSensorId: number | undefined
	dataDefinitionId: number | undefined
	log: DataLogClass | undefined
	createdAt: string | undefined
	updatedAt: string | undefined
	sensor: EquipmentClass | undefined

	constructor(model: DataLastValueInput = {}) {
		if (model) {
			Object.assign(this, model)
			if (model.log) {
				this.log = new DataLogClass(model.log)
			}
			if (model.sensor) {
				this.sensor = new EquipmentClass(model.sensor)
			}
		}
	}
}
