import { FunctionalityDefinitionClass } from './FunctionalityDefinitionClass'
import { ObjectDefinitionClass } from './ObjectDefinitionClass'
import { AccessLevelDefinitionClass } from './AccessLevelDefinitionClass'

export interface PermissionInput {
	id?: number | undefined
	userId?: number | undefined
	roleId?: number | undefined
	admFunctionalityDefinitionId?: number | undefined
	admObjectDefinitionId?: number | undefined
	admAccessLevelDefinitionId?: number | undefined
	functionalityDefinition?: FunctionalityDefinitionClass | undefined
	objectDefinition?: ObjectDefinitionClass | undefined
	accessLevelDefinition?: AccessLevelDefinitionClass | undefined
}

export class PermissionClass implements PermissionInput {
	id: number | undefined
	userId: number | undefined
	roleId: number | undefined
	admFunctionalityDefinitionId: number | undefined
	admObjectDefinitionId: number | undefined
	admAccessLevelDefinitionId: number | undefined
	functionalityDefinition: FunctionalityDefinitionClass | undefined
	objectDefinition: ObjectDefinitionClass | undefined
	accessLevelDefinition: AccessLevelDefinitionClass | undefined

	constructor(model: PermissionInput = {}) {
		if (model) {
			Object.assign(this, model)
			if (model.functionalityDefinition) {
				this.functionalityDefinition = new FunctionalityDefinitionClass(model.functionalityDefinition)
			}
			if (model.objectDefinition) {
				this.objectDefinition = new ObjectDefinitionClass(model.objectDefinition)
			}
			if (model.accessLevelDefinition) {
				this.accessLevelDefinition = new AccessLevelDefinitionClass(model.accessLevelDefinition)
			}
		}
	}
}
