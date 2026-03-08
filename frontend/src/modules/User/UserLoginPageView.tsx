import { redirect, useNavigate } from 'react-router'
import type { ILoginData } from './scripts/UserInterface'
import { Typography, Paper, Container } from '@mui/material'
import UserLoginForm from './components/UserLoginForm'
import { loginAction } from '../../store/account-actions'
import { showAlert } from '../../store/application-store'
import { useAppDispatch } from '../../store/hooks'
import { useLoginMutation } from '../../store/api/userApi'

export default function LoginPageView() {
	const dispatch = useAppDispatch()
	const navigate = useNavigate()
	const [login] = useLoginMutation()

	async function logIn(loginData: ILoginData): Promise<void> {
		try {
			const response = await login(loginData).unwrap()
			if (response && response.token && response.permissionToken) {
				localStorage.setItem('token', response.token)
				localStorage.setItem('permissionToken', response.permissionToken)
				dispatch(loginAction({ token: response.token, permissionToken: response.permissionToken }))
				dispatch(showAlert({ message: 'User logged in', severity: 'success' }))
				setTimeout(() => navigate('/'), 100)
			}
		} catch (err: any) {
			let message: string | string[] =
				err?.data?.message ||
				err?.message ||
				(err?.data && Array.isArray(err.data) && err.data.map((e: any) => e.msg).join(', ')) ||
				'Login failed'

			dispatch(showAlert({ message, severity: 'error' }))
		}
	}

	return (
		<Container maxWidth='xs' sx={{ mt: { xs: 4, sm: 8 } }}>
			<Paper
				elevation={0}
				sx={{
					p: { xs: 3, sm: 4 },
					textAlign: 'center',
					border: '1px solid',
					borderColor: 'divider',
				}}>
				<Typography variant='h5' sx={{ mb: 1, fontWeight: 700 }}>
					Welcome back
				</Typography>
				<Typography variant='body2' sx={{ mb: 3, color: 'text.secondary' }}>
					Sign in to your account
				</Typography>
				<UserLoginForm logIn={logIn} />
			</Paper>
		</Container>
	)
}

export function loader(): Response | undefined {
	if (localStorage.getItem('token')) {
		return redirect('/')
	}
}
