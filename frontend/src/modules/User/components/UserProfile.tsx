import { useState, lazy } from 'react'
import type { IUserProfileProps, IUserProfileData } from '../scripts/IUser'
const UserAvatar = lazy(() => import('./UserAvatar'))
import UserForm from './UserForm'
import { useAppDispatch } from '../../../store/hooks'
import { showAlert } from '../../../store/application-store'
import { useUpdateUserMutation } from '../../../store/api/userApi'
import { Paper, Typography } from '@mui/material'

export default function UserProfile({ user }: IUserProfileProps) {
	const [avatar, setAvatar] = useState<File | null>(null)
	const dispatch = useAppDispatch()
	const [updateUser] = useUpdateUserMutation()

	function handleAvatarChange(newAvatar: File): void {
		setAvatar(newAvatar)
	}

	async function handleSaveChanges(data: IUserProfileData): Promise<void> {
		const formData = new FormData()
		if (data.username) formData.append('username', data.username)
		if (data.email) formData.append('email', data.email)
		if (data.password) formData.append('password', data.password)
		if (avatar) formData.append('avatar', avatar)
		if (user.id) {
			try {
				await updateUser({ body: formData, id: user.id }).unwrap()
				dispatch(showAlert({ message: 'Profile data saved successfully', severity: 'success' }))
				setAvatar(null)
			} catch (err: any) {
				const message = err?.data?.message || err?.message || 'Something went wrong'
				dispatch(showAlert({ message, severity: 'error' }))
			}
		}
	}

	return (
		<Paper
			elevation={0}
			sx={{
				p: { xs: 3, sm: 4 },
				mt: 2,
				maxWidth: 500,
				mx: 'auto',
				border: '1px solid',
				borderColor: 'divider',
			}}>
			<Typography variant='h6' sx={{ fontWeight: 700, mb: 2 }}>
				Your profile
			</Typography>
			<UserAvatar avatarUrl={user.avatar} onAvatarChange={handleAvatarChange} />
			<UserForm user={user} onSave={handleSaveChanges} />
		</Paper>
	)
}
