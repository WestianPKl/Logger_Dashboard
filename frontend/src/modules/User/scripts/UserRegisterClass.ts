export interface UserRegiserInput {
	username?: string | undefined
	email?: string | undefined
	password?: string | undefined
	confirmPassword?: string | undefined
}

export class UserRegisterClass implements UserRegiserInput {
	username: string | undefined
	email: string | undefined
	password: string | undefined
	confirmPassword: string | undefined

	constructor(model: UserRegiserInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
