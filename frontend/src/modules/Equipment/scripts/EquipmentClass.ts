import { UserClass } from '../../User/scripts/UserClass'
import { EquipmentModelClass } from './EquipmentModelClass'
import { EquipmentTypeClass } from './EquipmentTypeClass'
import { EquipmentVendorClass } from './EquipmentVendorClass'
import { DataLastValueClass } from '../../Data/scripts/DataLastValueClass'
import type { DataDefinitionClass } from '../../Data/scripts/DataDefinitionClass'
import { EquipmentStatsClass } from './EquipmentStats'
import type { EquipmentLogClass } from './EquipmentLogs'
import type { ErrorLogClass } from './ErrorLog'

export interface EquipmentInput {
	id?: number | undefined
	serialNumber?: string | undefined
	equVendorId?: number | undefined
	equModelId?: number | undefined
	equTypeId?: number | undefined
	createdById?: number | undefined
	updatedById?: number | undefined
	createdAt?: string | undefined
	updatedAt?: string | undefined
	vendor?: EquipmentVendorClass | undefined
	model?: EquipmentModelClass | undefined
	type?: EquipmentTypeClass | undefined
	stats?: EquipmentStatsClass | undefined
	logs?: EquipmentLogClass[] | undefined
	errors?: ErrorLogClass[] | undefined
	dataDefinitions?: DataDefinitionClass[]
	lastValue?: DataLastValueClass | undefined
	createdBy?: UserClass | undefined
	updatedBy?: UserClass | undefined
}

export class EquipmentClass implements EquipmentInput {
	id: number | undefined
	serialNumber: string | undefined
	equVendorId: number | undefined
	equModelId: number | undefined
	equTypeId: number | undefined
	createdById: number | undefined
	updatedById: number | undefined
	createdAt: string | undefined
	updatedAt: string | undefined
	vendor: EquipmentVendorClass | undefined
	model: EquipmentModelClass | undefined
	type: EquipmentTypeClass | undefined
	dataDefinitions: DataDefinitionClass[] = []
	stats: EquipmentStatsClass | undefined
	logs: EquipmentLogClass[] = []
	errors: ErrorLogClass[] = []
	lastValue: DataLastValueClass | undefined
	createdBy: UserClass | undefined
	updatedBy: UserClass | undefined

	constructor(model: EquipmentInput = {}) {
		if (model) {
			Object.assign(this, model)
			if (model.vendor) {
				this.vendor = new EquipmentVendorClass(model.vendor)
			}
			if (model.model) {
				this.model = new EquipmentModelClass(model.model)
			}
			if (model.type) {
				this.type = new EquipmentTypeClass(model.type)
			}
			if (model.stats) {
				this.stats = new EquipmentStatsClass(model.stats)
			}
			if (model.lastValue) {
				this.lastValue = new DataLastValueClass(model.lastValue)
			}
			if (model.createdBy) {
				this.createdBy = new UserClass(model.createdBy)
			}
			if (model.updatedBy) {
				this.updatedBy = new UserClass(model.updatedBy)
			}
		}
	}
}
