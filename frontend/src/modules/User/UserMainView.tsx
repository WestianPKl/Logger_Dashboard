import * as React from 'react'
import { Outlet, useNavigate, useLocation, redirect } from 'react-router'
import { Tabs, Tab, Box } from '@mui/material'
import { store } from '../../store/store'
import { showAlert } from '../../store/application-store'

export default function UserMainView() {
	const navigate = useNavigate()
	const location = useLocation()

	const validTabs = ['user', 'permissions', 'roles']
	const tab = location.pathname.split('/').pop() || validTabs[0]
	let value = validTabs.indexOf(tab)
	if (value === -1) value = 0

	function handleChange(_: React.SyntheticEvent, newValue: number): void {
		const tabPath = validTabs[newValue]
		navigate(tabPath)
	}

	return (
		<>
			<Box sx={{ borderBottom: 1, borderColor: 'divider', bgcolor: 'background.paper', borderRadius: '12px 12px 0 0' }}>
				<Tabs
					value={value}
					onChange={handleChange}
					variant='scrollable'
					scrollButtons='auto'
					aria-label='horizontal-tab-user-profile'>
					{validTabs.map(label => (
						<Tab key={label} label={label.charAt(0).toUpperCase() + label.slice(1)} />
					))}
				</Tabs>
			</Box>

			<Box component='div' sx={{ mt: 3, width: '100%', textAlign: 'center', justifyContent: 'center' }}>
				<Outlet />
			</Box>
		</>
	)
}

export function loader(): Response | undefined {
	if (!localStorage.getItem('token')) {
		store.dispatch(showAlert({ message: 'Unknown user - token not found', severity: 'error' }))
		return redirect('/login')
	}
}
