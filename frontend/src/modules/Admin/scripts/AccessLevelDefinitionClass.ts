export interface AccessLevelDefinition {
	id: number | undefined
	name: string | undefined
	accessLevel: number | undefined
}

export class AccessLevelDefinitionClass implements AccessLevelDefinition {
	id: number | undefined
	name: string | undefined
	accessLevel: number | undefined

	constructor(model: Partial<AccessLevelDefinition> = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
