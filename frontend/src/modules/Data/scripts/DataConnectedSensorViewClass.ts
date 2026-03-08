export interface DataConnectedSensorViewInput {
	id?: number | undefined
	equLoggerId?: number | undefined
	equSensorId?: number | undefined
	houseFloorId?: number | undefined
	houseLoggerId?: number | undefined
	sensorVendor?: string | undefined
	sensorModel?: string | undefined
	sensorSerialNumber?: string | undefined
}

export class DataConnectedSensorViewClass implements DataConnectedSensorViewInput {
	id: number | undefined
	equLoggerId: number | undefined
	equSensorId: number | undefined
	houseFloorId: number | undefined
	houseLoggerId: number | undefined
	sensorVendor: string | undefined
	sensorModel: string | undefined
	sensorSerialNumber: string | undefined

	constructor(model: DataConnectedSensorViewInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
