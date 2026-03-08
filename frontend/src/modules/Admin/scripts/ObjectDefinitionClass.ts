export interface ObjectDefinitionInput {
	id?: number | undefined
	name?: string | undefined
	description?: string | undefined
}

export class ObjectDefinitionClass implements ObjectDefinitionInput {
	id: number | undefined
	name: string | undefined
	description: string | undefined

	constructor(model: ObjectDefinitionInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
