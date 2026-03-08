export interface ErrorLogsInput {
	id?: number | undefined
	message?: string | undefined
	details?: string | undefined
	type?: 'Equipment' | 'DB' | 'Other' | undefined
	severity?: 'Critical' | 'Error' | 'Warning' | 'Info' | undefined
	equipmentId?: number | undefined
	createdAt?: string | undefined
}

export class ErrorLogClass implements ErrorLogsInput {
	id: number | undefined
	message: string | undefined
	details: string | undefined
	type: 'Equipment' | 'DB' | 'Other' | undefined
	severity: 'Critical' | 'Error' | 'Warning' | 'Info' | undefined
	equipmentId: number | undefined
	createdAt: string | undefined

	constructor(model: ErrorLogsInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
