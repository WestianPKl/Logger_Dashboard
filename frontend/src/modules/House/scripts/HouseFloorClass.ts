import { HouseClass } from './HouseClass'
import { HouseLoggerClass } from './HouseLoggerClass'

export interface HouseFloorInput {
	id?: number | undefined
	name?: string | undefined
	layout?: string | undefined
	layoutBig?: string | undefined
	houseId?: number | undefined
	house?: HouseClass | undefined
	loggers?: HouseLoggerClass[]
	x?: number
	y?: number
	zoom?: number
	posX?: number | undefined
	posY?: number | undefined
}

export class HouseFloorClass implements HouseFloorInput {
	id: number | undefined
	name: string | undefined
	layout: string | undefined
	layoutBig: string | undefined
	houseId: number | undefined
	house: HouseClass | undefined
	loggers: HouseLoggerClass[] = []
	x: number = 0
	y: number = 0
	zoom: number = 1
	posX: number | undefined
	posY: number | undefined

	constructor(model: HouseFloorInput = {}) {
		if (model) {
			Object.assign(this, model)
			if (model.house) {
				this.house = new HouseClass(model.house)
			}
		}
	}
}
