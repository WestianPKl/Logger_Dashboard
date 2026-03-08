import { useState } from 'react'
import { Button, Box, TextField, useMediaQuery, useTheme } from '@mui/material'
import type { IPasswordResetFormProps } from '../scripts/UserInterface'

type FormErrors = {
	password?: string
	confirmPassword?: string
}

export default function UserPasswordResetForm({ getPasswordReset }: IPasswordResetFormProps) {
	const [password, setPassword] = useState('')
	const [confirmPassword, setConfirmPassword] = useState('')
	const [errors, setErrors] = useState<FormErrors>({})

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	function validate(): FormErrors {
		const newErrors: FormErrors = {}
		if (!password) newErrors.password = 'Password is required'
		if (!confirmPassword) newErrors.confirmPassword = 'Please confirm password'
		return newErrors
	}

	function onSubmitHandler(e: React.FormEvent): void {
		e.preventDefault()
		const validationErrors = validate()
		setErrors(validationErrors)
		if (Object.keys(validationErrors).length === 0) {
			getPasswordReset(password, confirmPassword)
		}
	}

	return (
		<Box onSubmit={onSubmitHandler} component='form' noValidate autoComplete='off' sx={{ width: '100%' }}>
			<Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
				<TextField
					fullWidth
					error={Boolean(errors.password)}
					id='password'
					label='Password'
					type='password'
					autoComplete='password'
					helperText={errors.password || ''}
					value={password}
					onChange={e => {
						setPassword(e.target.value)
						if (errors.password) setErrors({ ...errors, password: undefined })
					}}
				/>
				<TextField
					fullWidth
					error={Boolean(errors.confirmPassword)}
					id='confirm-password'
					label='Confirm password'
					type='password'
					autoComplete='confirm-password'
					helperText={errors.confirmPassword || ''}
					value={confirmPassword}
					onChange={e => {
						setConfirmPassword(e.target.value)
						if (errors.confirmPassword) setErrors({ ...errors, confirmPassword: undefined })
					}}
				/>
				<Button fullWidth variant='contained' size={isMobile ? 'medium' : 'large'} type='submit' sx={{ py: 1.2 }}>
					Reset password
				</Button>
			</Box>
		</Box>
	)
}
