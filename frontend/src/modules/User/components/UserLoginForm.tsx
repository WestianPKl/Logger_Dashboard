import { useState } from 'react'
import { useNavigate } from 'react-router'
import { Button, Box, TextField, useMediaQuery, useTheme } from '@mui/material'
import type { ILoginFormProps } from '../scripts/UserInterface'

export default function UserLoginForm({ logIn }: ILoginFormProps) {
	const [username, setUsername] = useState('')
	const [password, setPassword] = useState('')
	const [errors, setErrors] = useState<{ username?: string; password?: string }>({})
	const navigate = useNavigate()

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	function onSubmitHandler(e: React.FormEvent): void {
		e.preventDefault()
		const newErrors: { username?: string; password?: string } = {}
		if (!username) newErrors.username = 'Username is required'
		if (!password) newErrors.password = 'Password is required'
		setErrors(newErrors)
		if (Object.keys(newErrors).length === 0) {
			logIn({ username, password })
		}
	}

	function passwordReset(): void {
		navigate('/password-reset')
	}

	return (
		<Box onSubmit={onSubmitHandler} component='form' noValidate autoComplete='off' sx={{ width: '100%' }}>
			<Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
				<TextField
					fullWidth
					error={!!errors.username}
					id='login-username'
					label='Username'
					value={username}
					autoComplete='username'
					helperText={errors.username || ''}
					onChange={e => {
						setUsername(e.target.value)
						if (errors.username) setErrors({ ...errors, username: undefined })
					}}
				/>
				<TextField
					fullWidth
					error={!!errors.password}
					id='login-password'
					label='Password'
					type='password'
					autoComplete='password'
					value={password}
					helperText={errors.password || ''}
					onChange={e => {
						setPassword(e.target.value)
						if (errors.password) setErrors({ ...errors, password: undefined })
					}}
				/>
				<Button
					fullWidth
					variant='contained'
					size={isMobile ? 'medium' : 'large'}
					type='submit'
					sx={{ mt: 1, py: 1.2 }}>
					Sign in
				</Button>
				<Button
					fullWidth
					variant='text'
					size='small'
					type='button'
					sx={{ color: 'text.secondary' }}
					onClick={passwordReset}>
					Forgot password?
				</Button>
			</Box>
		</Box>
	)
}
