import { api } from './api'
import type { DataDefinitionClass } from '../../modules/Data/scripts/DataDefinitionClass'
import type { DataLogClass } from '../../modules/Data/scripts/DataLogClass'
import type { DataLastValueClass } from '../../modules/Data/scripts/DataLastValueClass'
import { DataLastValueViewClass } from '../../modules/Data/scripts/DataLastValueViewClass'
import { DataConnectedSensorViewClass } from '../../modules/Data/scripts/DataConnectedSensorViewClass'
import { DataLogsViewClass } from '../../modules/Data/scripts/DataLogsViewClass'

export const dataApi = api.injectEndpoints({
	endpoints: build => ({
		getDataDefinitions: build.query<DataDefinitionClass[], any>({
			query: (body: any) => ({
				url: 'api/data/data-definitions',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: DataDefinitionClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['DataDefinition'],
		}),
		getDataDefinition: build.query<DataDefinitionClass, number>({
			query: (id: number) => ({
				url: `api/data/data-definition/${id}`,
			}),
			transformResponse: (response: { data: DataDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['DataDefinition'],
		}),
		addDataDefinition: build.mutation<DataDefinitionClass, any>({
			query: (body: any) => ({
				url: 'api/data/data-definition',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: DataDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['DataDefinition'],
		}),
		updateDataDefinition: build.mutation<DataDefinitionClass, any>({
			query: (body: any) => ({
				url: `api/data/data-definition/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: DataDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['DataDefinition'],
		}),
		deleteDataDefinition: build.mutation<DataDefinitionClass, any>({
			query: (body: any) => ({
				url: `api/data/data-definition/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: DataDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['DataDefinition'],
		}),
		getDataLogs: build.query<DataLogClass[], any>({
			query: (body: any) => ({
				url: 'api/data/data-logs',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: DataLogClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['DataLog'],
		}),
		getDataLog: build.query<DataLogClass, number>({
			query: (id: number) => ({
				url: `api/data/data-log/${id}`,
			}),
			transformResponse: (response: { data: DataLogClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['DataLog'],
		}),
		addDataLog: build.mutation<DataLogClass, any>({
			query: (body: any) => ({
				url: 'api/data/data-log',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: DataLogClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['DataLog'],
		}),
		updateDataLog: build.mutation<DataLogClass, any>({
			query: (body: any) => ({
				url: `api/data/data-log/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: DataLogClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['DataLog'],
		}),
		deleteDataLog: build.mutation<DataLogClass, any>({
			query: (body: any) => ({
				url: `api/data/data-log/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: DataLogClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['DataLog'],
		}),
		getDataLastValues: build.query<DataLastValueClass[], any>({
			query: (body: any) => ({
				url: 'api/data/data-last-values',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: DataLastValueClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['DataLastValue'],
		}),
		getDataLastValue: build.query<DataLastValueClass, number>({
			query: (id: number) => ({
				url: `api/data/data-last-value/${id}`,
			}),
			transformResponse: (response: { data: DataLastValueClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['DataLastValue'],
		}),
		addDataLastValue: build.mutation<DataLastValueClass, any>({
			query: (body: any) => ({
				url: 'api/data/data-last-value',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: DataLastValueClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['DataLastValue'],
		}),
		updateDataLastValue: build.mutation<DataLastValueClass, any>({
			query: (body: any) => ({
				url: `api/data/data-last-value/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: DataLastValueClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['DataLastValue'],
		}),
		deleteDataLastValue: build.mutation<DataLastValueClass, any>({
			query: (body: any) => ({
				url: `api/data/data-last-value/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: DataLastValueClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['DataLastValue'],
		}),

		getDataLastValuesView: build.query<DataLastValueViewClass[], any>({
			query: (body: any) => ({
				url: 'api/data/data-last-values-view',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: DataLastValueViewClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['DataLastValue'],
		}),

		getDataLastValuesMulti: build.query<DataLastValueViewClass[], any>({
			query: (body: any) => ({
				url: 'api/data/data-last-values-multi',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: DataLastValueViewClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['DataLastValue'],
		}),
		getDataConnectedSensorView: build.query<DataConnectedSensorViewClass[], any>({
			query: (body: any) => ({
				url: 'api/data/data-connected-sensor-view',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: DataConnectedSensorViewClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['DataLastValue'],
		}),
		getDataLogsView: build.query<DataLogsViewClass[], any>({
			query: (body: any) => ({
				url: 'api/data/data-logs-view',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: DataLogsViewClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['DataLog'],
		}),
		getErrorProne: build.query<{ success: boolean }, void>({
			query: () => 'error-prone',
		}),
	}),
})

export const {
	useGetDataDefinitionsQuery,
	useGetDataDefinitionQuery,
	useAddDataDefinitionMutation,
	useUpdateDataDefinitionMutation,
	useDeleteDataDefinitionMutation,
	useGetDataLogsQuery,
	useGetDataLogQuery,
	useAddDataLogMutation,
	useUpdateDataLogMutation,
	useDeleteDataLogMutation,
	useGetDataLastValuesQuery,
	useGetDataLastValueQuery,
	useAddDataLastValueMutation,
	useUpdateDataLastValueMutation,
	useDeleteDataLastValueMutation,
	useGetDataLastValuesViewQuery,
	useGetDataConnectedSensorViewQuery,
	useGetDataLogsViewQuery,
	useGetDataLastValuesMultiQuery,
} = dataApi
