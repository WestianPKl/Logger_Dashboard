import { useState, useEffect, useRef } from 'react'
import { Button, Box } from '@mui/material'
import { CameraAlt } from '@mui/icons-material'
import type { IUserAvatarProps } from '../scripts/IUser'

export default function UserAvatar({ avatarUrl, onAvatarChange }: IUserAvatarProps) {
	const [previewImg, setPreviewImg] = useState<string | null>(
		avatarUrl ? `${import.meta.env.VITE_API_IP}/${avatarUrl}?w=150&h=150&format=webp` : null,
	)
	const inputRef = useRef<HTMLInputElement | null>(null)

	useEffect(() => {
		if (avatarUrl) {
			setPreviewImg(`${import.meta.env.VITE_API_IP}/${avatarUrl}?w=150&h=150&format=webp`)
		}
	}, [avatarUrl])

	function handleAvatarChange(e: React.ChangeEvent<HTMLInputElement>): void {
		const file = e.target.files?.[0]
		if (file) {
			const reader = new FileReader()
			reader.onloadend = () => {
				setPreviewImg(reader.result as string)
				onAvatarChange(file)
				if (inputRef.current) inputRef.current.value = ''
			}
			reader.readAsDataURL(file)
		}
	}

	return (
		<Box sx={{ mb: 3 }} display='flex' justifyContent='center' alignItems='center' flexDirection='column'>
			<Box
				sx={{
					width: 120,
					height: 120,
					mb: 2,
					borderRadius: '50%',
					overflow: 'hidden',
					border: '3px solid',
					borderColor: 'primary.light',
					boxShadow: '0 4px 16px rgba(74, 158, 158, 0.2)',
				}}>
				<img
					src={previewImg || ''}
					alt='User avatar'
					width={120}
					height={120}
					loading='lazy'
					style={{
						width: '100%',
						height: '100%',
						objectFit: 'cover',
						background: '#eee',
					}}
				/>
				{!previewImg && <span>U</span>}
			</Box>
			<Button variant='outlined' component='label' startIcon={<CameraAlt />} size='small'>
				Upload Avatar
				<input type='file' accept='image/*' hidden onChange={handleAvatarChange} ref={inputRef} />
			</Button>
		</Box>
	)
}
