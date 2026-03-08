import type { UserClass } from '../../User/scripts/UserClass'
import type { AccessLevelDefinitionClass } from './AccessLevelDefinitionClass'
import type { AdminRoleClass } from './AdminRoleClass'
import type { FunctionalityDefinitionClass } from './FunctionalityDefinitionClass'
import type { ObjectDefinitionClass } from './ObjectDefinitionClass'
import type { GridFilterModel, GridSortModel } from '@mui/x-data-grid'

export interface IAddAccessLevelDefinitionProps {
	edit: boolean
	selectedItems?: AccessLevelDefinitionClass[]
	handleCloseAdd: () => void
	openAddDialog: boolean
	addItemHandler: (item: IAddAccessLevelDefinitionData[] | IAddAccessLevelDefinitionData) => void
}

export interface IAddAccessLevelDefinitionData {
	id?: number
	name: string | undefined
	accessLevel: number | undefined
}

export interface IAccessLevelDefinitionTableProps {
	accessLevels: AccessLevelDefinitionClass[]
	initSort: GridSortModel
	initFilter: GridFilterModel
}

export interface IAddFunctionalityDefinitionProps {
	edit: boolean
	selectedItems?: FunctionalityDefinitionClass[]
	handleCloseAdd: () => void
	openAddDialog: boolean
	addItemHandler: (item: IAddFunctionalityDefinitionData[] | IAddFunctionalityDefinitionData) => void
}

export interface IAddFunctionalityDefinitionData {
	id?: number
	name: string | undefined
	description: string | undefined
}

export interface IFunctionalityDefinitionTableProps {
	functionalityDefinitions: FunctionalityDefinitionClass[]
	initSort: GridSortModel
	initFilter: GridFilterModel
}

export interface IAddObjectDefinitionProps {
	edit: boolean
	selectedItems?: ObjectDefinitionClass[]
	handleCloseAdd: () => void
	openAddDialog: boolean
	addItemHandler: (item: IAddObjectDefinitionData[] | IAddObjectDefinitionData) => void
}

export interface IAddObjectDefinitionData {
	id?: number
	name: string | undefined
	description: string | undefined
}

export interface IObjectDefinitionTableProps {
	objectDefinitions: ObjectDefinitionClass[]
	initSort: GridSortModel
	initFilter: GridFilterModel
}

export interface IAddAdminRoleProps {
	edit: boolean
	selectedItems?: AdminRoleClass[]
	handleCloseAdd: () => void
	openAddDialog: boolean
	addItemHandler: (item: IAddAdminRoleData[] | IAddAdminRoleData) => void
}

export interface IAdminRolesTableProps {
	admRoles: AdminRoleClass[]
	initSort: GridSortModel
	initFilter: GridFilterModel
}

export interface IAddAdminRoleData {
	id?: number
	name: string | undefined
	description: string | undefined
}

export interface IUserTableProps {
	users: UserClass[]
	initSort: GridSortModel
	initFilter: GridFilterModel
}

export interface IAddAdminRoleUserData {
	roleId: number | undefined
	userId: number | undefined
}

export interface IAddAdminRolePermissionData {
	id?: number
	userId?: number
	roleId?: number
	admFunctionalityDefinitionId: number | undefined
	admObjectDefinitionId: number | undefined | null
	admAccessLevelDefinitionId: number | undefined
}

export interface IAddAdminRoleUserProps {
	selectedItems: AdminRoleClass[]
	handleCloseAdd: () => void
	openAddDialog: boolean
}

export interface IAddAdminRolePermissionProps {
	selectedItems: AdminRoleClass[]
	handleCloseAdd: () => void
	openAddDialog: boolean
}

export interface IAddAdminUserPermissionProps {
	selectedItems: UserClass[]
	handleCloseAdd: () => void
	openAddDialog: boolean
}

export interface IAddAdminUserPermissionDialogProps {
	userId?: number
	roleId?: number
	handleCloseAdd: () => void
	openAddDialog: boolean
	addItemHandler: (item: IAddAdminRolePermissionData[] | IAddAdminRolePermissionData) => void
}

export interface IAddAdminRoleUserDataDialog {
	roleId: number | undefined
	user: UserClass[] | undefined
}

export interface IAddAdminRoleUserDialogProps {
	roleId: number
	handleCloseAdd: () => void
	openAddDialog: boolean
	addItemHandler: (item: IAddAdminRoleUserDataDialog[] | IAddAdminRoleUserDataDialog) => void
}

export interface IRoleUserPermissionProps {
	usersData: UserClass[]
	isAdmin?: boolean
	userId?: number
	roleId?: number
	initSort: GridSortModel
	initFilter: GridFilterModel
}
