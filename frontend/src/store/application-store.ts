import { createAppSlice } from './createAppSlice'
import type { PayloadAction } from '@reduxjs/toolkit'
import type { IApplicationState } from './scripts/StoreScripts'

const initialState: IApplicationState = {
	message: '',
	severity: 'success',
	isActive: false,
	timeout: 5000,
}

export const applicationSlice = createAppSlice({
	name: 'application',
	initialState,
	reducers: create => ({
		showAlert: create.reducer(
			(
				state,
				action: PayloadAction<{
					message: string | string[]
					severity: 'success' | 'info' | 'warning' | 'error'
					timeout?: number
				}>,
			) => {
				state.message = action.payload.message
				state.severity = action.payload.severity
				state.isActive = true
				if (action.payload.timeout !== undefined) {
					state.timeout = action.payload.timeout
				}
			},
		),
		hideAlert: create.reducer(state => {
			state.isActive = false
			state.message = ''
			state.severity = 'success'
		}),
	}),
	selectors: {
		selectTimeout: (state: IApplicationState) => state.timeout,
		selectIsActive: (state: IApplicationState) => state.isActive,
		selectMessage: (state: IApplicationState) => state.message,
		selectSeverity: (state: IApplicationState) => state.severity,
	},
})

export const { showAlert, hideAlert } = applicationSlice.actions
export const { selectTimeout, selectIsActive, selectMessage, selectSeverity } = applicationSlice.selectors
