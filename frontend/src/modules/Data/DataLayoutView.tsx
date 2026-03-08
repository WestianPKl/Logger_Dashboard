import { Outlet, redirect } from 'react-router'
import { store } from '../../store/store'
import { showAlert } from '../../store/application-store'

export default function DataLayoutView() {
	return <Outlet />
}

export function loader(): Response | undefined {
	if (!localStorage.getItem('token')) {
		store.dispatch(showAlert({ message: 'Unknown user - token not found', severity: 'error' }))
		return redirect('/login')
	}
}
