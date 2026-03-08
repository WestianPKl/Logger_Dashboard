import type { AdminRoleUserClass } from '../../Admin/scripts/AdminRoleUserClass'
import type { PermissionClass } from '../../Admin/scripts/PermissionClass'
import type { UserClass } from './UserClass'
import type { GridFilterModel, GridSortModel } from '@mui/x-data-grid'

export interface IUserProfileProps {
	user: UserClass
}

export interface IUserFormProps {
	user: UserClass
	onSave: (data: IUserProfileData) => void
}

export interface IUserProfileData {
	username: string | undefined
	email: string | undefined
	password: string | undefined
}

export interface IUserAvatarProps {
	avatarUrl: string | undefined
	onAvatarChange: (avatarData: any) => void
}

export interface IUserPermissionProps {
	permissionData: PermissionClass[]
	isAdmin?: boolean
	userId?: number
	roleId?: number
	initSort: GridSortModel
	initFilter: GridFilterModel
}

export interface IUserRolesProps {
	rolesData: AdminRoleUserClass[]
	initSort: GridSortModel
	initFilter: GridFilterModel
}
