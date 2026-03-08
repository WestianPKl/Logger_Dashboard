import { UserClass } from '../../User/scripts/UserClass'
import { ProcessTypeClass } from './ProcessTypeClass'

export interface ProcessDefinitionInput {
	id?: number | undefined
	processTypeId?: number | undefined
	name?: string | undefined
	createdById?: number | undefined
	updatedById?: number | undefined
	createdAt?: string | undefined
	updatedAt?: string | undefined
	type?: ProcessTypeClass | undefined
	createdBy?: UserClass | undefined
	updatedBy?: UserClass | undefined
}

export class ProcessDefinitionClass implements ProcessDefinitionInput {
	id: number | undefined
	processTypeId: number | undefined
	name: string | undefined
	createdById: number | undefined
	updatedById: number | undefined
	createdAt: string | undefined
	updatedAt: string | undefined
	type: ProcessTypeClass | undefined
	createdBy: UserClass | undefined
	updatedBy: UserClass | undefined

	constructor(model: ProcessDefinitionInput = {}) {
		if (model) {
			Object.assign(this, model)
			if (model.type) {
				this.type = new ProcessTypeClass(model.type)
			}
			if (model.createdBy) {
				this.createdBy = new UserClass(model.createdBy)
			}
			if (model.updatedBy) {
				this.updatedBy = new UserClass(model.updatedBy)
			}
		}
	}
}
