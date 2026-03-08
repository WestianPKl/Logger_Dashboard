import { UserClass } from '../../modules/User/scripts/UserClass'
import { PermissionClass } from '../../modules/Admin/scripts/PermissionClass'
import { AccessLevelDefinitionClass } from '../../modules/Admin/scripts/AccessLevelDefinitionClass'

export interface IAccountState {
	user: UserClass | undefined
	isLogged: boolean
	isAdmin: boolean
	loading: boolean
	duration: number | undefined
	token: string | undefined
	avatar: string | undefined
}
export interface IApplicationState {
	message: string[] | string
	severity: 'success' | 'info' | 'warning' | 'error'
	isActive: boolean
	timeout: number
}
export interface IAuthState {
	permissions: PermissionClass[]
	accessLevels: AccessLevelDefinitionClass[]
}
