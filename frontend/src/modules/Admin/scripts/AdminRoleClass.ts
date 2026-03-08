import { UserClass } from '../../User/scripts/UserClass'

export interface AdminRoleInput {
	id?: number
	name?: string
	description?: string
	createdById?: number
	updatedById?: number
	createdAt?: string
	updatedAt?: string
	createdBy?: UserClass
	updatedBy?: UserClass
	users?: UserClass[]
}

export class AdminRoleClass implements AdminRoleInput {
	id: number | undefined
	name: string | undefined
	description: string | undefined
	createdById: number | undefined
	updatedById: number | undefined
	createdAt: string | undefined
	updatedAt: string | undefined
	createdBy: UserClass | undefined
	updatedBy: UserClass | undefined
	users: UserClass[] = []

	constructor(model: AdminRoleInput = {}) {
		if (model) {
			Object.assign(this, model)
			if (model.createdBy) {
				this.createdBy = new UserClass(model.createdBy)
			}
			if (model.updatedBy) {
				this.updatedBy = new UserClass(model.updatedBy)
			}
		}
	}
}
