export interface FunctionalityDefinitionInput {
	id?: number | undefined
	name?: string | undefined
	description?: string | undefined
}

export class FunctionalityDefinitionClass implements FunctionalityDefinitionInput {
	id: number | undefined
	name: string | undefined
	description: string | undefined

	constructor(model: FunctionalityDefinitionInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
