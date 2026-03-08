export interface UserInput {
	id?: number | undefined
	username?: string | undefined
	email?: string | undefined
	createdAt?: string | undefined
	updatedAt?: string | undefined
	confirmed?: number | undefined
	avatar?: string | undefined
	avatarBig?: string | undefined
}

export class UserClass implements UserInput {
	id: number | undefined
	username: string | undefined
	email: string | undefined
	createdAt: string | undefined
	updatedAt: string | undefined
	confirmed: number | undefined
	avatar?: string | undefined
	avatarBig?: string | undefined

	constructor(model: UserInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
