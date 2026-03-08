export interface EquipmentUnsusedLoggerInput {
	equLoggerId?: number | undefined
}

export class EquipmentUnusedLoggerClass implements EquipmentUnsusedLoggerInput {
	equLoggerId: number | undefined

	constructor(model: EquipmentUnsusedLoggerInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
