import { api } from './api'
import type { ProcessDefinitionClass } from '../../modules/Process/scripts/ProcessDefinitionClass'
import type { ProcessTypeClass } from '../../modules/Process/scripts/ProcessTypeClass'

export const processApi = api.injectEndpoints({
	endpoints: build => ({
		getProcessTypes: build.query<ProcessTypeClass[], any>({
			query: (body: any) => ({
				url: 'api/process/process-types',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: ProcessTypeClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['ProcessType'],
		}),
		getProcessType: build.query<ProcessTypeClass, number>({
			query: (id: number) => ({
				url: `api/process/process-type/${id}`,
			}),
			transformResponse: (response: { data: ProcessTypeClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['ProcessType'],
		}),
		addProcessType: build.mutation<ProcessTypeClass, any>({
			query: (body: any) => ({
				url: 'api/process/process-type',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: ProcessTypeClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['ProcessType'],
		}),
		updateProcessType: build.mutation<ProcessTypeClass, any>({
			query: (body: any) => ({
				url: `api/process/process-type/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: ProcessTypeClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['ProcessType'],
		}),
		deleteProcessType: build.mutation<ProcessTypeClass, any>({
			query: (body: any) => ({
				url: `api/process/process-type/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: ProcessTypeClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['ProcessType'],
		}),

		getProcessDefinitions: build.query<ProcessDefinitionClass[], any>({
			query: (body: any) => ({
				url: 'api/process/process-definitions',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: ProcessDefinitionClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['ProcessDefinition'],
		}),
		getProcessDefinition: build.query<ProcessDefinitionClass, number>({
			query: (id: number) => ({
				url: `api/process/process-definition/${id}`,
			}),
			transformResponse: (response: { data: ProcessDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['ProcessDefinition'],
		}),
		addProcessDefinition: build.mutation<ProcessDefinitionClass, any>({
			query: (body: any) => ({
				url: 'api/process/process-definition',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: ProcessDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['ProcessDefinition'],
		}),
		updateProcessDefinition: build.mutation<ProcessDefinitionClass, any>({
			query: (body: any) => ({
				url: `api/process/process-definition/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: ProcessDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['ProcessDefinition'],
		}),
		deleteProcessDefinition: build.mutation<ProcessDefinitionClass, any>({
			query: (body: any) => ({
				url: `api/process/process-definition/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: ProcessDefinitionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['ProcessDefinition'],
		}),

		getErrorProne: build.query<{ success: boolean }, void>({
			query: () => 'error-prone',
		}),
	}),
})

export const {
	useGetProcessTypesQuery,
	useGetProcessTypeQuery,
	useAddProcessTypeMutation,
	useUpdateProcessTypeMutation,
	useDeleteProcessTypeMutation,
	useGetProcessDefinitionsQuery,
	useGetProcessDefinitionQuery,
	useAddProcessDefinitionMutation,
	useUpdateProcessDefinitionMutation,
	useDeleteProcessDefinitionMutation,
	useGetErrorProneQuery,
} = processApi
