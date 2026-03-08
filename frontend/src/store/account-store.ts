import type { PayloadAction } from '@reduxjs/toolkit'
import { createAppSlice } from './createAppSlice'
import type { IAccountState } from './scripts/StoreScripts'
import type { UserClass } from '../modules/User/scripts/UserClass'

const initialState: IAccountState = {
	user: undefined,
	isLogged: false,
	isAdmin: false,
	loading: false,
	duration: undefined,
	token: undefined,
	avatar: undefined,
}

export const accountSlice = createAppSlice({
	name: 'account',
	initialState,
	reducers: create => ({
		logIn: create.reducer((state, action: PayloadAction<{ expiration: number; user: UserClass; token: string }>) => {
			state.duration = action.payload.expiration
			state.isLogged = true
			state.user = action.payload.user
			state.loading = false
			state.token = action.payload.token
		}),
		logOff: create.reducer(state => {
			state.duration = undefined
			state.isAdmin = false
			state.isLogged = false
			state.loading = false
			state.user = undefined
			state.token = undefined
			state.avatar = undefined
		}),
		getAuthDuration: create.reducer((state, action: PayloadAction<number>) => {
			state.duration = action.payload
		}),
		getUserAvatar: create.reducer((state, action: PayloadAction<string | undefined>) => {
			state.avatar = action.payload
		}),
	}),
	selectors: {
		selectIsLogged: state => state.isLogged,
		selectUser: state => state.user,
		selectDuration: state => state.duration,
		selectToken: state => state.token,
		selectAvatar: state => state.avatar,
	},
})

export const { logIn, logOff, getAuthDuration, getUserAvatar } = accountSlice.actions
export const { selectIsLogged, selectUser, selectDuration, selectToken, selectAvatar } = accountSlice.selectors
