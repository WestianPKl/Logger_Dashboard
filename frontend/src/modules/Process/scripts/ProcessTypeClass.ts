export interface ProcessTypeInput {
	id?: number | undefined
	name?: string | undefined
}

export class ProcessTypeClass implements ProcessTypeInput {
	id: number | undefined
	name: string | undefined

	constructor(model: ProcessTypeInput = {}) {
		if (model) {
			Object.assign(this, model)
		}
	}
}
