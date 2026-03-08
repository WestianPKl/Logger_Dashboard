import { Link, useSubmit, useLocation } from 'react-router'
import { ListItem, ListItemButton, ListItemIcon, ListItemText } from '@mui/material'
import type { IAppDrawerItemProps } from '../scripts/AppInterface'

export default function AppDrawerItem({ text, icon, link }: IAppDrawerItemProps) {
	const submit = useSubmit()
	const location = useLocation()
	const isActive = link ? location.pathname === link || location.pathname.startsWith(link + '/') : false

	const activeStyle = isActive
		? {
				backgroundColor: 'rgba(74, 158, 158, 0.12)',
				color: 'primary.dark',
				'& .MuiListItemIcon-root': { color: 'primary.dark' },
			}
		: {}

	return (
		<ListItem disablePadding>
			{text === 'Logout' ? (
				<ListItemButton onClick={() => submit(null, { action: '/logout', method: 'post' })} sx={{ py: 1.2 }}>
					<ListItemIcon sx={{ minWidth: 40, color: 'text.secondary' }}>{icon}</ListItemIcon>
					<ListItemText primary={text} primaryTypographyProps={{ fontWeight: 500, fontSize: '0.9rem' }} />
				</ListItemButton>
			) : (
				<ListItemButton component={Link} to={link} sx={{ py: 1.2, ...activeStyle }}>
					<ListItemIcon sx={{ minWidth: 40, color: isActive ? 'primary.dark' : 'text.secondary' }}>{icon}</ListItemIcon>
					<ListItemText
						primary={text}
						primaryTypographyProps={{ fontWeight: isActive ? 600 : 500, fontSize: '0.9rem' }}
					/>
				</ListItemButton>
			)}
		</ListItem>
	)
}
