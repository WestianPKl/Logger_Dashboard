export interface DataLastValueViewInput {
	id?: number | undefined
	equLoggerId?: number | undefined
	equSensorId?: number | undefined
	houseFloorId?: number | undefined
	houseLoggerId?: number | undefined
	time?: string | undefined
	value?: number | undefined
	parameter?: string | undefined
	unit?: string | undefined
}

export class DataLastValueViewClass implements DataLastValueViewInput {
	id: number | undefined
	equLoggerId: number | undefined
	equSensorId: number | undefined
	houseFloorId: number | undefined
	houseLoggerId: number | undefined
	time: string | undefined
	value: number | undefined
	parameter: string | undefined
	unit: string | undefined

	constructor(model: DataLastValueViewInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
