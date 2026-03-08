import { lazy, Suspense } from 'react'
import LoadingCircle from '../../components/UI/LoadingCircle'
import { redirect, useNavigate } from 'react-router'
import { Typography, Paper, Container } from '@mui/material'
const RegisterForm = lazy(() => import('./components/UserRegisterForm'))
import type { IRegisterData } from './scripts/UserInterface'
import type { IErrorData } from '../Application/scripts/AppInterface'
import { useAppDispatch } from '../../store/hooks'
import { showAlert } from '../../store/application-store'
import { useRegisterMutation } from '../../store/api/userApi'

export default function RegisterPageView() {
	const dispatch = useAppDispatch()
	const navigate = useNavigate()
	const [register] = useRegisterMutation()

	async function createNewAccount(registerData: IRegisterData): Promise<void> {
		try {
			await register(registerData).unwrap()
			dispatch(showAlert({ message: 'Account created successfully', severity: 'success' }))
			navigate('/login')
		} catch (err: any) {
			let errorArray: string[] = []
			const errorData = err?.data?.data
			if (Array.isArray(errorData) && errorData.length > 0) {
				errorArray = errorData.map((e: IErrorData) => e.msg)
			} else if (err?.data?.message) {
				errorArray = [err.data.message]
			} else if (err?.error) {
				errorArray = [err.error]
			} else {
				errorArray = ['Registration failed. Please try again.']
			}
			dispatch(showAlert({ message: errorArray, severity: 'error' }))
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
					Create account
				</Typography>
				<Typography variant='body2' sx={{ mb: 3, color: 'text.secondary' }}>
					Register a new account
				</Typography>
				<Suspense fallback={<LoadingCircle />}>
					<RegisterForm createNewAccount={createNewAccount} />
				</Suspense>
			</Paper>
		</Container>
	)
}

export function loader(): Response | undefined {
	if (localStorage.getItem('token')) {
		return redirect('/')
	}
}
