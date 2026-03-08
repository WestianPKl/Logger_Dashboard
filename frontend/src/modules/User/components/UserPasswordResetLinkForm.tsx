import { useState } from 'react'
import { Button, Box, TextField, useMediaQuery, useTheme } from '@mui/material'
import type { IPasswordResetLinkFormProps } from '../scripts/UserInterface'

type FormErrors = {
	email?: string
}

export default function UserPasswordResetLinkForm({ getPasswordResetLink }: IPasswordResetLinkFormProps) {
	const [email, setEmail] = useState('')
	const [errors, setErrors] = useState<FormErrors>({})

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	function validate(): FormErrors {
		const newErrors: FormErrors = {}
		if (!email) newErrors.email = 'Email is required'
		else if (!/^[\w-.]+@([\w-]+\.)+[\w-]{2,4}$/.test(email)) newErrors.email = 'Invalid email'
		return newErrors
	}

	function onSubmitHandler(e: React.FormEvent): void {
		e.preventDefault()
		const validationErrors = validate()
		setErrors(validationErrors)
		if (Object.keys(validationErrors).length === 0) {
			getPasswordResetLink(email)
		}
	}

	return (
		<Box onSubmit={onSubmitHandler} component='form' noValidate autoComplete='off' sx={{ width: '100%' }}>
			<Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
				<TextField
					fullWidth
					error={Boolean(errors.email)}
					id='email'
					label='Email'
					type='email'
					autoComplete='email'
					helperText={errors.email || ''}
					value={email}
					onChange={e => {
						setEmail(e.target.value)
						if (errors.email) setErrors({ ...errors, email: undefined })
					}}
				/>
				<Button fullWidth variant='contained' size={isMobile ? 'medium' : 'large'} type='submit' sx={{ py: 1.2 }}>
					Send reset link
				</Button>
			</Box>
		</Box>
	)
}
