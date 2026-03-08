import { useEffect } from 'react'
import type { UserClass } from '../modules/User/scripts/UserClass'
import { TextField, Autocomplete, useMediaQuery, useTheme } from '@mui/material'
import { useGetUsersQuery } from '../store/api/userApi'
import { useAppDispatch } from '../store/hooks'
import { showAlert } from '../store/application-store'

interface ISelectProps {
	getItem: (item: UserClass[]) => void
	item: UserClass[] | undefined
}

export default function AdminUserSelect({ getItem, item }: ISelectProps) {
	const dispatch = useAppDispatch()

	const { data: users = [], error: usersError } = useGetUsersQuery({})

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	useEffect(() => {
		if (usersError) {
			const message = (usersError as any)?.data?.message || (usersError as any)?.message || 'Something went wrong'
			dispatch(showAlert({ message, severity: 'error' }))
		}
	}, [dispatch, usersError])

	function getOptionLabel(user: UserClass): string {
		return user.username && user.email ? `${user.username} ${user.email}` : ''
	}

	return (
		<Autocomplete
			multiple
			limitTags={2}
			sx={{ mt: '1rem', width: isMobile ? 200 : 400 }}
			onChange={(_, value) => getItem(value)}
			disablePortal
			value={item ?? []}
			getOptionLabel={getOptionLabel}
			options={users}
			slotProps={{ listbox: { sx: { maxHeight: '100px' } } }}
			renderInput={params => <TextField {...params} label='Users' />}
		/>
	)
}
