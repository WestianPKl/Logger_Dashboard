import { redirect, useNavigate } from 'react-router'
import { Typography, Paper, Container } from '@mui/material'
import UserPasswordResetLinkForm from './components/UserPasswordResetLinkForm'
import { showAlert } from '../../store/application-store'
import { useAppDispatch } from '../../store/hooks'
import { usePasswordResetTokenMutation } from '../../store/api/userApi'

export default function UserPasswordResetLinkView() {
	const dispatch = useAppDispatch()
	const navigate = useNavigate()
	const [getLink] = usePasswordResetTokenMutation()

	async function getPasswordResetLink(email: string | undefined): Promise<void> {
		try {
			const response = await getLink({ email: email }).unwrap()
			if (response) {
				dispatch(showAlert({ message: 'Password reset mail sent', severity: 'success' }))
				navigate('/login')
			}
		} catch (err: any) {
			let message: string | string[] =
				err?.data?.message ||
				err?.message ||
				(err?.data && Array.isArray(err.data) && err.data.map((e: any) => e.msg).join(', ')) ||
				'Password reset failed'

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
					Password reset
				</Typography>
				<Typography variant='body2' sx={{ mb: 3, color: 'text.secondary' }}>
					Enter your email to receive a reset link
				</Typography>
				<UserPasswordResetLinkForm getPasswordResetLink={getPasswordResetLink} />
			</Paper>
		</Container>
	)
}

export function loader(): Response | undefined {
	if (localStorage.getItem('token')) {
		return redirect('/')
	}
}
