export interface EquipmentVendorInput {
	id?: number | undefined
	name?: string | undefined
}

export class EquipmentVendorClass implements EquipmentVendorInput {
	id: number | undefined
	name: string | undefined

	constructor(model: EquipmentVendorInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
