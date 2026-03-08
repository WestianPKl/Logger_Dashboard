import { api } from './api'
import type { UserLoginClass } from '../../modules/User/scripts/UserLoginClass'
import type { ILoginData, IRegisterData } from '../../modules/User/scripts/UserInterface'
import type { UserClass } from '../../modules/User/scripts/UserClass'

export const userApi = api.injectEndpoints({
	endpoints: build => ({
		login: build.mutation<UserLoginClass, ILoginData>({
			query: (credentials: ILoginData) => ({
				url: 'api/user/user-login',
				method: 'POST',
				body: credentials,
			}),
			transformResponse: (response: { data: UserLoginClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['User'],
		}),
		register: build.mutation<UserClass, IRegisterData>({
			query: (credentials: IRegisterData) => ({
				url: 'api/user/user-register',
				method: 'POST',
				body: credentials,
			}),
			transformResponse: (response: { data: UserClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['User'],
		}),
		getUsers: build.query<UserClass[], any>({
			query: (body: any) => ({
				url: `api/user/users`,
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: UserClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['UserData'],
		}),
		getUser: build.query<UserClass, number>({
			query: (id: number) => ({
				url: `api/user/user/${id}`,
				method: 'GET',
			}),
			transformResponse: (response: { data: UserClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['UserData'],
		}),

		updateUser: build.mutation<UserClass, { body: any; id: number }>({
			query: ({ body, id }: { body: any; id: number }) => ({
				url: `api/user/user/${id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: UserClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['UserData'],
		}),

		passwordResetToken: build.mutation<{ email: string }, any>({
			query: (body: any) => ({
				url: 'api/user/reset-password-request',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: { email: string } }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['User'],
		}),

		passwordReset: build.mutation<{ email: string }, any>({
			query: (body: any) => ({
				url: `api/user/reset-password/${body.token}`,
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: { email: string } }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['User'],
		}),
		getErrorProne: build.query<{ success: boolean }, void>({
			query: () => 'error-prone',
		}),
	}),
})

export const {
	useLoginMutation,
	useRegisterMutation,
	useGetErrorProneQuery,
	useGetUsersQuery,
	useGetUserQuery,
	useUpdateUserMutation,
	usePasswordResetTokenMutation,
	usePasswordResetMutation,
} = userApi

export const {
	endpoints: { login, register, getUser },
} = userApi
