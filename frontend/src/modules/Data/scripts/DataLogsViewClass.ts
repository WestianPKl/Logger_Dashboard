export interface DataLogsViewInput {
	time?: string | undefined
	equLoggerId?: number | undefined
	equSensorId?: number | undefined
	temperature?: string | undefined
	humidity?: string | undefined
	atmPressure?: string | undefined
	altitude?: string | undefined
}

export class DataLogsViewClass implements DataLogsViewInput {
	time: string | undefined
	equLoggerId: number | undefined
	equSensorId: number | undefined
	temperature: string | undefined
	humidity: string | undefined
	atmPressure: string | undefined
	altitude: string | undefined

	constructor(model: DataLogsViewInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
