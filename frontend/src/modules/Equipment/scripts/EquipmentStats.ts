export interface EquipmentStatsInput {
	id?: number | undefined
	equipmentId?: number | undefined
	lastSeen?: string | undefined
	snContr?: string | undefined
	fwContr?: string | undefined
	hwContr?: string | undefined
	buildContr?: string | undefined
	prodContr?: string | undefined
	snCom?: string | undefined
	fwCom?: string | undefined
	hwCom?: string | undefined
	buildCom?: string | undefined
	prodCom?: string | undefined
	ipAddress?: string | undefined
}

export class EquipmentStatsClass implements EquipmentStatsInput {
	id: number | undefined
	equipmentId: number | undefined
	lastSeen: string | undefined
	snContr: string | undefined
	fwContr: string | undefined
	hwContr: string | undefined
	buildContr: string | undefined
	prodContr: string | undefined
	snCom: string | undefined
	fwCom: string | undefined
	hwCom: string | undefined
	buildCom: string | undefined
	prodCom: string | undefined
	ipAddress: string | undefined

	constructor(model: EquipmentStatsInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
