import { api } from './api'
import type { EquipmentClass } from '../../modules/Equipment/scripts/EquipmentClass'
import type { EquipmentModelClass } from '../../modules/Equipment/scripts/EquipmentModelClass'
import type { EquipmentVendorClass } from '../../modules/Equipment/scripts/EquipmentVendorClass'
import type { EquipmentTypeClass } from '../../modules/Equipment/scripts/EquipmentTypeClass'
import type { IAddEquipment, IAddEquipmentData } from '../../modules/Equipment/scripts/IEquipment'
import type { EquipmentUnusedLoggerClass } from '../../modules/Equipment/scripts/EquipmentUnusedLoggerClass'
import type { EquipmentSensorFunctionClass } from '../../modules/Equipment/scripts/EquipmentSensorFunctions'

export const equipmentApi = api.injectEndpoints({
	endpoints: build => ({
		getEquipments: build.query<EquipmentClass[], any>({
			query: (body: any) => ({
				url: 'api/equipment/equipments',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: EquipmentClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['Equipment'],
		}),
		getEquipmentsAdmin: build.query<EquipmentClass[], any>({
			query: (body: any) => ({
				url: 'api/equipment/equipments-admin',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: EquipmentClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['Equipment'],
		}),
		getEquipment: build.query<EquipmentClass, number>({
			query: (id: number) => ({
				url: `api/equipment/equipment/${id}`,
			}),
			transformResponse: (response: { data: EquipmentClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['Equipment'],
		}),
		addEquipment: build.mutation<EquipmentClass, IAddEquipment>({
			query: (body: IAddEquipment) => ({
				url: 'api/equipment/equipment',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: EquipmentClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['Equipment'],
		}),
		updateEquipment: build.mutation<EquipmentClass, IAddEquipment>({
			query: (body: IAddEquipment) => ({
				url: `api/equipment/equipment/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: EquipmentClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['Equipment'],
		}),
		deleteEquipment: build.mutation<EquipmentClass, any>({
			query: (body: any) => ({
				url: `api/equipment/equipment/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: EquipmentClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['Equipment'],
		}),
		restoreEquipment: build.mutation<EquipmentClass, IAddEquipment>({
			query: (body: IAddEquipment) => ({
				url: `api/equipment/equipment-restore/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: EquipmentClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['Equipment'],
		}),

		getEquipmentModels: build.query<EquipmentModelClass[], any>({
			query: (body: any) => ({
				url: 'api/equipment/equ-models',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: EquipmentModelClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['EquipmentModel'],
		}),
		getEquipmentModel: build.query<EquipmentModelClass, number>({
			query: (id: number) => ({
				url: `api/equipment/equ-model/${id}`,
			}),
			transformResponse: (response: { data: EquipmentModelClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['EquipmentModel'],
		}),
		addEquipmentModel: build.mutation<EquipmentModelClass, IAddEquipmentData>({
			query: (body: IAddEquipmentData) => ({
				url: 'api/equipment/equ-model',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: EquipmentModelClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['EquipmentModel'],
		}),
		updateEquipmentModel: build.mutation<EquipmentModelClass, IAddEquipmentData>({
			query: (body: IAddEquipmentData) => ({
				url: `api/equipment/equ-model/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: EquipmentModelClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['EquipmentModel'],
		}),
		deleteEquipmentModel: build.mutation<EquipmentModelClass, any>({
			query: (body: any) => ({
				url: `api/equipment/equ-model/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: EquipmentModelClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['EquipmentModel'],
		}),
		getEquipmentVendors: build.query<EquipmentVendorClass[], any>({
			query: (body: any) => ({
				url: 'api/equipment/equ-vendors',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: EquipmentVendorClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['EquipmentVendor'],
		}),
		getEquipmentVendor: build.query<EquipmentVendorClass, number>({
			query: (id: number) => ({
				url: `api/equipment/equ-vendor/${id}`,
			}),
			transformResponse: (response: { data: EquipmentVendorClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['EquipmentVendor'],
		}),
		addEquipmentVendor: build.mutation<EquipmentVendorClass, IAddEquipmentData>({
			query: (body: IAddEquipmentData) => ({
				url: 'api/equipment/equ-vendor',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: EquipmentVendorClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['EquipmentVendor'],
		}),
		updateEquipmentVendor: build.mutation<EquipmentVendorClass, IAddEquipmentData>({
			query: (body: IAddEquipmentData) => ({
				url: `api/equipment/equ-vendor/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: EquipmentVendorClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['EquipmentVendor'],
		}),
		deleteEquipmentVendor: build.mutation<EquipmentVendorClass, any>({
			query: (body: any) => ({
				url: `api/equipment/equ-vendor/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: EquipmentVendorClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['EquipmentVendor'],
		}),

		getEquipmentTypes: build.query<EquipmentTypeClass[], any>({
			query: (body: any) => ({
				url: 'api/equipment/equ-types',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: EquipmentTypeClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['EquipmentType'],
		}),
		getEquipmentType: build.query<EquipmentTypeClass, number>({
			query: (id: number) => ({
				url: `api/equipment/equ-type/${id}`,
			}),
			transformResponse: (response: { data: EquipmentTypeClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['EquipmentType'],
		}),
		addEquipmentType: build.mutation<EquipmentTypeClass, IAddEquipmentData>({
			query: (body: IAddEquipmentData) => ({
				url: 'api/equipment/equ-type',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: EquipmentTypeClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['EquipmentType'],
		}),
		updateEquipmentType: build.mutation<EquipmentVendorClass, IAddEquipmentData>({
			query: (body: IAddEquipmentData) => ({
				url: `api/equipment/equ-type/${body.id}`,
				method: 'PATCH',
				body,
			}),
			transformResponse: (response: { data: EquipmentTypeClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['EquipmentType'],
		}),
		deleteEquipmentType: build.mutation<EquipmentTypeClass, any>({
			query: (body: any) => ({
				url: `api/equipment/equ-type/${body.id}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: EquipmentTypeClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['EquipmentType'],
		}),

		getEquipmentUnusedLoggers: build.query<EquipmentUnusedLoggerClass[], any>({
			query: (body: any) => ({
				url: 'api/equipment/equ-unused-loggers',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: EquipmentUnusedLoggerClass[] }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			providesTags: ['Equipment'],
		}),
		addEquipmentSensorFunction: build.mutation<EquipmentSensorFunctionClass, any>({
			query: (body: any) => ({
				url: 'api/house/equ-sensor-function',
				method: 'POST',
				body,
			}),
			transformResponse: (response: { data: EquipmentSensorFunctionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['House'],
		}),
		deleteEquipmentSensorFunction: build.mutation<EquipmentSensorFunctionClass, any>({
			query: (body: any) => ({
				url: `api/house/equ-sensor-function/${body.equSensorId}`,
				method: 'DELETE',
				body,
			}),
			transformResponse: (response: { data: EquipmentSensorFunctionClass }) => response.data,
			transformErrorResponse: (response: { status: number; data: { message: string } }) => {
				return { status: response.status, message: response.data.message }
			},
			invalidatesTags: ['House'],
		}),
		getErrorProne: build.query<{ success: boolean }, void>({
			query: () => 'error-prone',
		}),
	}),
})

export const {
	useGetEquipmentsQuery,
	useGetEquipmentsAdminQuery,
	useGetEquipmentQuery,
	useAddEquipmentMutation,
	useUpdateEquipmentMutation,
	useDeleteEquipmentMutation,
	useRestoreEquipmentMutation,
	useGetEquipmentModelsQuery,
	useGetEquipmentModelQuery,
	useAddEquipmentModelMutation,
	useUpdateEquipmentModelMutation,
	useDeleteEquipmentModelMutation,
	useGetEquipmentVendorsQuery,
	useGetEquipmentVendorQuery,
	useAddEquipmentVendorMutation,
	useUpdateEquipmentVendorMutation,
	useDeleteEquipmentVendorMutation,
	useGetEquipmentTypesQuery,
	useGetEquipmentTypeQuery,
	useAddEquipmentTypeMutation,
	useUpdateEquipmentTypeMutation,
	useDeleteEquipmentTypeMutation,
	useGetEquipmentUnusedLoggersQuery,
	useAddEquipmentSensorFunctionMutation,
	useDeleteEquipmentSensorFunctionMutation,
	useGetErrorProneQuery,
} = equipmentApi
