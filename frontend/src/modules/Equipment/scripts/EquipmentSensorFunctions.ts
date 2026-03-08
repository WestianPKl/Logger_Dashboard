export interface EquipmentSensorFunctionInput {
	equSensorId?: number | undefined
	dataDefinitionId?: number | undefined
}

export class EquipmentSensorFunctionClass implements EquipmentSensorFunctionInput {
	equSensorId: number | undefined
	dataDefinitionId: number | undefined

	constructor(model: EquipmentSensorFunctionInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
