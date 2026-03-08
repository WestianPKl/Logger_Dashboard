export interface EquipmentTypeInput {
	id?: number | undefined
	name?: string | undefined
}

export class EquipmentTypeClass implements EquipmentTypeInput {
	id: number | undefined
	name: string | undefined

	constructor(model: EquipmentTypeInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
