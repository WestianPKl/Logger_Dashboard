import { EquipmentClass } from '../../Equipment/scripts/EquipmentClass'
import { HouseFloorClass } from './HouseFloorClass'

export interface HouseLoggerInput {
	id?: number | undefined
	equLoggerId?: number | undefined
	houseFloorId?: number | undefined
	posX?: number | undefined
	posY?: number | undefined
	logger?: EquipmentClass | undefined
	floor?: HouseFloorClass | undefined
}

export class HouseLoggerClass implements HouseLoggerInput {
	id: number | undefined
	equLoggerId: number | undefined
	houseFloorId: number | undefined
	posX: number | undefined
	posY: number | undefined
	logger: EquipmentClass | undefined
	floor: HouseFloorClass | undefined

	constructor(model: HouseLoggerInput = {}) {
		if (model) {
			Object.assign(this, model)
			if (model.logger) {
				this.logger = new EquipmentClass(model.logger)
			}
			if (model.floor) {
				this.floor = new HouseFloorClass(model.floor)
			}
		}
	}
}
