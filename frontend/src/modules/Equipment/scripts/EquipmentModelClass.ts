export interface EquipmentModelInput {
	id?: number | undefined
	name?: string | undefined
}

export class EquipmentModelClass implements EquipmentModelInput {
	id: number | undefined
	name: string | undefined

	constructor(model: EquipmentModelInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
