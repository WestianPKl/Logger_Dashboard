export interface UserLoginInput {
	token?: string | undefined
	permissionToken?: string | undefined
}

export class UserLoginClass implements UserLoginInput {
	token: string | undefined
	permissionToken: string | undefined

	constructor(model: UserLoginInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
