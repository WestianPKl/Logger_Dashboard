import { Tabs, Tab, Box } from '@mui/material'
import { Outlet, useNavigate, useLocation, redirect } from 'react-router'
import { store } from '../../store/store'
import { showAlert } from '../../store/application-store'

export default function AdminMainView() {
	const navigate = useNavigate()
	const location = useLocation()
	const validTabs = [
		'functionality-defnition',
		'object-definition',
		'access-levels-definition',
		'permission-roles',
		'users',
		'equipment',
		'equipment-vendors',
		'equipment-models',
		'equipment-types',
		'data-definitions',
	]
	const tab = location.pathname.split('/').pop() || validTabs[0]
	let value = validTabs.indexOf(tab)
	if (value === -1) value = 0

	const handleChange = (_: React.SyntheticEvent, newValue: number) => {
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
						<Tab key={label} label={label.replace(/-/g, ' ')} />
					))}
				</Tabs>
			</Box>

			<Box component='div' sx={{ mt: 3, width: '100%', textAlign: 'center', justifyContent: 'center' }}>
				<Outlet />
			</Box>
		</>
	)
}

export function loader() {
	if (!localStorage.getItem('token')) {
		store.dispatch(showAlert({ message: 'Unknown user - token not found', severity: 'error' }))
		return redirect('/login')
	}
}
