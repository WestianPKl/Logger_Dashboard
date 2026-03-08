import { createApi, fetchBaseQuery, retry } from '@reduxjs/toolkit/query/react'
import type { BaseQueryFn, FetchArgs, FetchBaseQueryError } from '@reduxjs/toolkit/query/react'

const baseQuery = fetchBaseQuery({
	baseUrl: import.meta.env.VITE_API_IP,
	prepareHeaders: headers => {
		const token = localStorage.getItem('token')
		if (token) {
			headers.set('Authorization', `Bearer ${token}`)
		} else {
			headers.delete('Authorization')
		}
		return headers
	},
})

const baseQueryWithAuth: BaseQueryFn<string | FetchArgs, unknown, FetchBaseQueryError> = async (
	args,
	api,
	extraOptions,
) => {
	const result = await baseQuery(args, api, extraOptions)

	if (result.error && result.error.status === 401) {
		localStorage.removeItem('token')
		localStorage.removeItem('permissionToken')
		window.location.href = '/login'
	}

	return result
}

const baseQueryWithRetry = retry(baseQueryWithAuth, { maxRetries: 0 })

export const api = createApi({
	reducerPath: 'splitApi',
	baseQuery: baseQueryWithRetry as BaseQueryFn<string | FetchArgs, unknown, FetchBaseQueryError>,
	tagTypes: [
		'User',
		'UserData',
		'Equipment',
		'EquipmentModel',
		'EquipmentVendor',
		'EquipmentType',
		'ProcessType',
		'ProcessDefinition',
		'AdminPermission',
		'AdminFunctionality',
		'AdminObject',
		'AdminAccessLevel',
		'AdminRole',
		'AdminRoleUser',
		'House',
		'HouseFloor',
		'HouseLogger',
		'DataDefinition',
		'DataLog',
		'DataLastValue',
	],
	endpoints: () => ({}),
})
