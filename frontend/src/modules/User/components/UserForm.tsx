import { useState, useEffect } from 'react'
import { Button, Box, TextField, useMediaQuery, useTheme } from '@mui/material'
import type { IUserFormProps } from '../scripts/IUser'

export default function UserForm({ user, onSave }: IUserFormProps) {
	const [username, setUsername] = useState(user.username ?? '')
	const [email, setEmail] = useState(user.email ?? '')
	const [password, setPassword] = useState('')
	const [errors, setErrors] = useState<{ username?: string; email?: string; password?: string }>({})

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		setUsername(user.username ?? '')
		setEmail(user.email ?? '')
	}, [user.username, user.email])

	function validate(): { username?: string; email?: string; password?: string } {
		const newErrors: typeof errors = {}
		if (!username) newErrors.username = 'Username is required'
		if (!email) newErrors.email = 'Email is required'
		else if (!/^[\w-.]+@([\w-]+\.)+[\w-]{2,4}$/.test(email)) newErrors.email = 'Invalid email'
		if (password && password.length < 3) newErrors.password = 'Password too short'
		return newErrors
	}

	function onSubmitHandler(e: React.FormEvent): void {
		e.preventDefault()
		const validation = validate()
		setErrors(validation)
		if (Object.keys(validation).length === 0) {
			onSave({ username, email, password })
			setPassword('')
		}
	}

	return (
		<Box onSubmit={onSubmitHandler} component='form' noValidate autoComplete='off' sx={{ width: '100%' }}>
			<Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
				<TextField
					fullWidth
					error={!!errors.username}
					id='username'
					label='Username'
					autoComplete='username'
					helperText={errors.username || ''}
					value={username}
					onChange={e => {
						setUsername(e.target.value)
						if (errors.username) setErrors({ ...errors, username: undefined })
					}}
				/>
				<TextField
					fullWidth
					error={!!errors.email}
					id='email'
					label='Email'
					type='email'
					autoComplete='email'
					value={email}
					helperText={errors.email || ''}
					onChange={e => {
						setEmail(e.target.value)
						if (errors.email) setErrors({ ...errors, email: undefined })
					}}
				/>
				<TextField
					fullWidth
					error={!!errors.password}
					id='password'
					label='Password'
					type='password'
					autoComplete='new-password'
					helperText={errors.password || 'Leave blank to keep current password'}
					value={password}
					onChange={e => {
						setPassword(e.target.value)
						if (errors.password) setErrors({ ...errors, password: undefined })
					}}
				/>

				<Button
					fullWidth
					variant='contained'
					color='primary'
					size={isMobile ? 'medium' : 'large'}
					type='submit'
					sx={{ mt: 1, py: 1.2 }}>
					Save Changes
				</Button>
			</Box>
		</Box>
	)
}
