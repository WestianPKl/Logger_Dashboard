import { api } from './api'
import type { PermissionClass } from '../../modules/Admin/scripts/PermissionClass'
import type { FunctionalityDefinitionClass } from '../../modules/Admin/scripts/FunctionalityDefinitionClass'
import type { ObjectDefinitionClass } from '../../modules/Admin/scripts/ObjectDefinitionClass'
import type { AccessLevelDefinitionClass } from '../../modules/Admin/scripts/AccessLevelDefinitionClass'
import type { AdminRoleClass } from '../../modules/Admin/scripts/AdminRoleClass'
import type { AdminRoleUserClass } from '../../modules/Admin/scripts/AdminRoleUserClass'
import type {
	IAddAccessLevelDefinitionData,
	IAddFunctionalityDefinitionData,
	IAddObjectDefinitionData,
} from '../../modules/Admin/scripts/IAdmin'

export const adminApi = api.injectEndpoints({
	endpoints: build => ({
		getPermissions: build.query<PermissionClass[], any>({
			query: (body: any) => ({
				url: 'api/adm/adm-permissions',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: PermissionClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['AdminPermission'],
		}),
		addPermission: build.mutation<PermissionClass, any>({
			query: (body: any) => ({
				url: 'api/adm/adm-permission',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: PermissionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminPermission'],
		}),
		updatePermission: build.mutation<PermissionClass, any>({
			query: (body: any) => ({
				url: `api/adm/adm-permission/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: PermissionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminPermission'],
		}),
		deletePermission: build.mutation<PermissionClass, any>({
			query: (body: any) => ({
				url: `api/adm/adm-permission/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: PermissionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminPermission'],
		}),
		getFunctionalityDefinitions: build.query<FunctionalityDefinitionClass[], any>({
			query: (body: any) => ({
				url: 'api/adm/adm-functionality-definitions',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: FunctionalityDefinitionClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['AdminFunctionality'],
		}),
		getFunctionalityDefinition: build.query<FunctionalityDefinitionClass, number>({
			query: (id: number) => ({
				url: `api/adm/adm-functionality-definition/${id}`,
			}),
			transformResponse: (response: { data: FunctionalityDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['AdminFunctionality'],
		}),
		addFunctionalityDefinition: build.mutation<FunctionalityDefinitionClass, IAddFunctionalityDefinitionData>({
			query: (body: IAddFunctionalityDefinitionData) => ({
				url: 'api/adm/adm-functionality-definition',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: FunctionalityDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminFunctionality'],
		}),
		updateFunctionalityDefinition: build.mutation<FunctionalityDefinitionClass, IAddFunctionalityDefinitionData>({
			query: (body: IAddFunctionalityDefinitionData) => ({
				url: `api/adm/adm-functionality-definition/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: FunctionalityDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminFunctionality'],
		}),
		deleteFunctionalityDefinition: build.mutation<FunctionalityDefinitionClass, any>({
			query: (body: any) => ({
				url: `api/adm/adm-functionality-definition/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: FunctionalityDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminFunctionality'],
		}),
		getObjectDefinitions: build.query<ObjectDefinitionClass[], any>({
			query: (body: any) => ({
				url: 'api/adm/adm-object-definitions',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: ObjectDefinitionClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['AdminObject'],
		}),
		getObjectDefinition: build.query<ObjectDefinitionClass, number>({
			query: (id: number) => ({
				url: `api/adm/adm-object-definition/${id}`,
			}),
			transformResponse: (response: { data: ObjectDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['AdminObject'],
		}),
		addObjectDefinition: build.mutation<ObjectDefinitionClass, IAddObjectDefinitionData>({
			query: (body: IAddObjectDefinitionData) => ({
				url: 'api/adm/adm-object-definition',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: ObjectDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminObject'],
		}),
		updateObjectDefinition: build.mutation<ObjectDefinitionClass, IAddObjectDefinitionData>({
			query: (body: IAddObjectDefinitionData) => ({
				url: `api/adm/adm-object-definition/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: ObjectDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminObject'],
		}),
		deleteObjectDefinition: build.mutation<ObjectDefinitionClass, any>({
			query: (body: any) => ({
				url: `api/adm/adm-object-definition/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: ObjectDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminObject'],
		}),
		getAccessLevelDefinitions: build.query<AccessLevelDefinitionClass[], any>({
			query: (body: any) => ({
				url: 'api/adm/adm-access-level-definitions',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: AccessLevelDefinitionClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['AdminAccessLevel'],
		}),
		getAccessLevelDefinition: build.query<AccessLevelDefinitionClass, number>({
			query: (id: number) => ({
				url: `api/adm/adm-access-level-definition/${id}`,
			}),
			transformResponse: (response: { data: AccessLevelDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['AdminAccessLevel'],
		}),
		addAccessLevelDefinition: build.mutation<AccessLevelDefinitionClass, IAddAccessLevelDefinitionData>({
			query: (body: IAddAccessLevelDefinitionData) => ({
				url: 'api/adm/adm-access-level-definition',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: AccessLevelDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminAccessLevel'],
		}),
		updateAccessLevelDefinition: build.mutation<AccessLevelDefinitionClass, any>({
			query: (body: any) => ({
				url: `api/adm/adm-access-level-definition/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: AccessLevelDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminAccessLevel'],
		}),
		deleteAccessLevelDefinition: build.mutation<AccessLevelDefinitionClass, any>({
			query: (body: any) => ({
				url: `api/adm/adm-access-level-definition/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: AccessLevelDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminAccessLevel'],
		}),
		getAdminRoles: build.query<AdminRoleClass[], any>({
			query: (body: any) => ({
				url: 'api/adm/adm-roles',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: AdminRoleClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['AdminRole'],
		}),
		getAdminRole: build.query<AdminRoleClass, number>({
			query: (id: number) => ({
				url: `api/adm/adm-role/${id}`,
			}),
			transformResponse: (response: { data: AdminRoleClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['AdminRole'],
		}),
		addAdminRole: build.mutation<AdminRoleClass, any>({
			query: (body: any) => ({
				url: 'api/adm/adm-role',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: AdminRoleClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminRole'],
		}),
		updateAdminRole: build.mutation<AdminRoleClass, any>({
			query: (body: any) => ({
				url: `api/adm/adm-role/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: AdminRoleClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminRole'],
		}),
		deleteAdminRole: build.mutation<AdminRoleClass, any>({
			query: (body: any) => ({
				url: `api/adm/adm-role/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: AdminRoleClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminRole'],
		}),

		getAdminRoleUsers: build.query<AdminRoleUserClass[], any>({
			query: (body: any) => ({
				url: 'api/adm/adm-role-users',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: AdminRoleUserClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['AdminRoleUser'],
		}),
		addAdminRoleUser: build.mutation<AdminRoleUserClass, any>({
			query: (body: any) => ({
				url: 'api/adm/adm-role-user',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: AdminRoleUserClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminRoleUser'],
		}),
		deleteAdminRoleUser: build.mutation<AdminRoleUserClass, any>({
			query: (body: any) => ({
				url: `api/adm/adm-role-user/${body.admRoleId}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: AdminRoleUserClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['AdminRoleUser'],
		}),

		getErrorProne: build.query<{ success: boolean }, void>({
			query: () => 'error-prone',
		}),
	}),
})

export const {
	useGetPermissionsQuery,
	useAddPermissionMutation,
	useUpdatePermissionMutation,
	useDeletePermissionMutation,
	useGetFunctionalityDefinitionsQuery,
	useGetFunctionalityDefinitionQuery,
	useAddFunctionalityDefinitionMutation,
	useUpdateFunctionalityDefinitionMutation,
	useDeleteFunctionalityDefinitionMutation,
	useGetObjectDefinitionsQuery,
	useGetObjectDefinitionQuery,
	useAddObjectDefinitionMutation,
	useUpdateObjectDefinitionMutation,
	useDeleteObjectDefinitionMutation,
	useGetAccessLevelDefinitionsQuery,
	useGetAccessLevelDefinitionQuery,
	useAddAccessLevelDefinitionMutation,
	useUpdateAccessLevelDefinitionMutation,
	useDeleteAccessLevelDefinitionMutation,
	useGetAdminRolesQuery,
	useGetAdminRoleQuery,
	useAddAdminRoleMutation,
	useUpdateAdminRoleMutation,
	useDeleteAdminRoleMutation,
	useGetAdminRoleUsersQuery,
	useAddAdminRoleUserMutation,
	useDeleteAdminRoleUserMutation,
	useGetErrorProneQuery,
} = adminApi

export const {
	endpoints: { getAdminRoleUsers, getAccessLevelDefinitions, getPermissions },
} = adminApi
