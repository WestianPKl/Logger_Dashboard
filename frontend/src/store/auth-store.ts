import { createAppSlice } from './createAppSlice'
import type { IAuthState } from './scripts/StoreScripts'
import type { PermissionClass } from '../modules/Admin/scripts/PermissionClass'
import type { PayloadAction } from '@reduxjs/toolkit'
import type { AccessLevelDefinitionClass } from '../modules/Admin/scripts/AccessLevelDefinitionClass'

const initialState: IAuthState = {
	permissions: [],
	accessLevels: [],
}

export const authenticateSlice = createAppSlice({
	name: 'authenticate',
	initialState,
	reducers: create => ({
		getPermissions: create.reducer((state, action: PayloadAction<PermissionClass[]>) => {
			state.permissions = action.payload
		}),
		getAccessLevels: create.reducer((state, action: PayloadAction<AccessLevelDefinitionClass[]>) => {
			state.accessLevels = action.payload
		}),
		clearAuthState: create.reducer(state => {
			state.permissions = []
			state.accessLevels = []
		}),
	}),
	selectors: {
		selectPermissions: (state: IAuthState) => state.permissions,
		selectAccessLevels: (state: IAuthState) => state.accessLevels,
	},
})

export const { getPermissions, getAccessLevels, clearAuthState } = authenticateSlice.actions
export const { selectPermissions, selectAccessLevels } = authenticateSlice.selectors
