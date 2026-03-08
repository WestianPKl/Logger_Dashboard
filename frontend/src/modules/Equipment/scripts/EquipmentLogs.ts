export interface EquipmentLogsInput {
	id?: number | undefined
	equipmentId?: number | undefined
	message?: string | undefined
	type?: 'STATUS' | 'DATA' | 'OTA' | 'ERROR' | 'OTHER' | undefined
	createdAt?: string | undefined
	updatedAt?: string | undefined
}

export class EquipmentLogClass implements EquipmentLogsInput {
	id: number | undefined
	equipmentId: number | undefined
	message: string | undefined
	type: 'STATUS' | 'DATA' | 'OTA' | 'ERROR' | 'OTHER' | undefined
	createdAt: string | undefined
	updatedAt: string | undefined

	constructor(model: EquipmentLogsInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
