import { api } from './api'
import type { HouseClass } from '../../modules/House/scripts/HouseClass'
import type { HouseFloorClass } from '../../modules/House/scripts/HouseFloorClass'
import type { HouseLoggerClass } from '../../modules/House/scripts/HouseLoggerClass'
import type { IAddHouseLoggerData } from '../../modules/House/scripts/IHouse'

export const houseApi = api.injectEndpoints({
	endpoints: build => ({
		getHouses: build.query<HouseClass[], any>({
			query: (body: any) => ({
				url: 'api/house/houses',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: HouseClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['House'],
		}),
		getHouse: build.query<HouseClass, number>({
			query: (id: number) => ({
				url: `api/house/house/${id}`,
			}),
			transformResponse: (response: { data: HouseClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['House'],
		}),
		addHouse: build.mutation<HouseClass, any>({
			query: (body: any) => ({
				url: 'api/house/house',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: HouseClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['House'],
		}),
		updateHouse: build.mutation<HouseClass, { body: any; id: number }>({
			query: ({ body, id }: { body: any; id: number }) => ({
				url: `api/house/house/${id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: HouseClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['House'],
		}),
		deleteHouse: build.mutation<HouseClass, any>({
			query: (body: any) => ({
				url: `api/house/house/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: HouseClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['House'],
		}),

		getHouseFloors: build.query<HouseFloorClass[], any>({
			query: (body: any) => ({
				url: 'api/house/house-floors',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: HouseFloorClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['HouseFloor'],
		}),
		getHouseFloor: build.query<HouseFloorClass, number>({
			query: (id: number) => ({
				url: `api/house/house-floor/${id}`,
			}),
			transformResponse: (response: { data: HouseFloorClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['HouseFloor'],
		}),
		addHouseFloor: build.mutation<HouseFloorClass, any>({
			query: (body: any) => ({
				url: 'api/house/house-floor',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: HouseFloorClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['House', 'HouseFloor'],
		}),
		updateHouseFloor: build.mutation<HouseFloorClass, { body: any; id: number }>({
			query: ({ body, id }: { body: any; id: number }) => ({
				url: `api/house/house-floor/${id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: HouseFloorClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['House', 'HouseFloor'],
		}),

		updateHouseFloorLayout: build.mutation<HouseFloorClass, { body: any; id: number }>({
			query: ({ body, id }: { body: any; id: number }) => ({
				url: `api/house/house-floor-layout/${id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: HouseFloorClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['House', 'HouseFloor'],
		}),
		deleteHouseFloor: build.mutation<HouseFloorClass, any>({
			query: (body: any) => ({
				url: `api/house/house-floor/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: HouseFloorClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['House', 'HouseFloor'],
		}),

		getHouseLoggers: build.query<HouseLoggerClass[], any>({
			query: (body: any) => ({
				url: 'api/house/house-loggers',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: HouseLoggerClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['HouseLogger'],
		}),
		getHouseLogger: build.query<HouseLoggerClass, number>({
			query: (id: number) => ({
				url: `api/house/house-logger/${id}`,
			}),
			transformResponse: (response: { data: HouseLoggerClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['HouseLogger'],
		}),
		addHouseLogger: build.mutation<HouseLoggerClass, IAddHouseLoggerData>({
			query: (body: IAddHouseLoggerData) => ({
				url: 'api/house/house-logger',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: HouseLoggerClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['House', 'HouseLogger'],
		}),
		updateHouseLogger: build.mutation<HouseLoggerClass, IAddHouseLoggerData>({
			query: (body: IAddHouseLoggerData) => ({
				url: `api/house/house-logger/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: HouseLoggerClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['House', 'HouseLogger'],
		}),
		deleteHouseLogger: build.mutation<HouseLoggerClass, any>({
			query: (body: any) => ({
				url: `api/house/house-logger/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: HouseLoggerClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['House', 'HouseLogger'],
		}),
		getErrorProne: build.query<{ success: boolean }, void>({
			query: () => 'error-prone',
		}),
	}),
})

export const {
	useGetHousesQuery,
	useGetHouseQuery,
	useAddHouseMutation,
	useUpdateHouseMutation,
	useDeleteHouseMutation,
	useGetHouseFloorsQuery,
	useGetHouseFloorQuery,
	useAddHouseFloorMutation,
	useUpdateHouseFloorMutation,
	useUpdateHouseFloorLayoutMutation,
	useDeleteHouseFloorMutation,
	useGetHouseLoggersQuery,
	useGetHouseLoggerQuery,
	useAddHouseLoggerMutation,
	useUpdateHouseLoggerMutation,
	useDeleteHouseLoggerMutation,
	useGetErrorProneQuery,
} = houseApi
