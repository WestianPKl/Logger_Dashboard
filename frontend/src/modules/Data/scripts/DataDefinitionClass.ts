export interface DataDefinitionInput {
	id?: number | undefined
	name?: string | undefined
	unit?: string | undefined
	description?: string | undefined
	createdAt?: string | undefined
	updatedAt?: string | undefined
}

export class DataDefinitionClass implements DataDefinitionInput {
	id: number | undefined
	name: string | undefined
	unit: string | undefined
	description: string | undefined
	createdAt: string | undefined
	updatedAt: string | undefined

	constructor(model: DataDefinitionInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
