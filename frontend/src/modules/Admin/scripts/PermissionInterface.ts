import type { PermissionClass } from './PermissionClass'

export interface IDecodedToken {
	tokenType: number
	user: { id: number; username: string; email: string; createdAt: string; updatedAt: string; confirmed: number }
	permissions: PermissionClass[]
	expiration: string
	superuser: boolean
}
