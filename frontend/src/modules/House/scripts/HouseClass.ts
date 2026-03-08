import { UserClass } from '../../User/scripts/UserClass'
import { HouseFloorClass } from './HouseFloorClass'

export interface HouseInput {
	id?: number | undefined
	name?: string | undefined
	postalCode?: string | undefined
	city?: string | undefined
	street?: string | undefined
	houseNumber?: string | undefined
	pictureLink?: string | undefined
	pictureLinkBig?: string | undefined
	createdById?: number | undefined
	updatedById?: number | undefined
	createdAt?: string | undefined
	updatedAt?: string | undefined
	floors?: HouseFloorClass[]
	createdBy?: UserClass | undefined
	updatedBy?: UserClass | undefined
}

export class HouseClass implements HouseInput {
	id: number | undefined
	name: string | undefined
	postalCode: string | undefined
	city: string | undefined
	street: string | undefined
	houseNumber: string | undefined
	pictureLink: string | undefined
	pictureLinkBig: string | undefined
	createdById: number | undefined
	updatedById: number | undefined
	createdAt: string | undefined
	updatedAt: string | undefined
	floors: HouseFloorClass[] = []
	createdBy: UserClass | undefined
	updatedBy: UserClass | undefined

	constructor(model: HouseInput = {}) {
		if (model) {
			Object.assign(this, model)
			if (model.createdBy) {
				this.createdBy = new UserClass(model.createdBy)
			}
			if (model.updatedBy) {
				this.updatedBy = new UserClass(model.updatedBy)
			}
		}
	}
}
