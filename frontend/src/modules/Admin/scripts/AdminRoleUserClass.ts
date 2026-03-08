import { AdminRoleClass } from './AdminRoleClass'

export interface AdminRoleUserInput {
	roleId?: number | undefined
	userId?: number | undefined
	role?: AdminRoleClass | undefined
}

export class AdminRoleUserClass implements AdminRoleUserInput {
	roleId: number | undefined
	userId: number | undefined
	role: AdminRoleClass | undefined

	constructor(model: AdminRoleUserInput = {}) {
		if (model) {
			Object.assign(this, model)
			if (model.role) {
				this.role = new AdminRoleClass(model.role)
			}
		}
	}
}
