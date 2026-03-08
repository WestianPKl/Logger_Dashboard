import { Box, List, Divider, Typography, useMediaQuery, useTheme } from '@mui/material'
import AppDrawerItem from './AppDrawerItem'
import type { IAppDrawerListProps, IAppDrawerArray } from '../scripts/AppInterface'
import HomeIcon from '@mui/icons-material/Home'
import HouseIcon from '@mui/icons-material/House'
import DeviceThermostatIcon from '@mui/icons-material/DeviceThermostat'
import StorageIcon from '@mui/icons-material/Storage'
import AdminPanelSettingsIcon from '@mui/icons-material/AdminPanelSettings'
import AccountBoxIcon from '@mui/icons-material/AccountBox'
import LogoutIcon from '@mui/icons-material/Logout'
import { useAppSelector } from '../../../store/hooks'
import { canRead } from '../../../store/auth-actions'
import { useMemo } from 'react'

export default function AppDrawerList({ toggleDrawer }: IAppDrawerListProps) {
	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	const canReadHouse = useAppSelector(state => canRead('house', null)(state))
	const canReadEquipment = useAppSelector(state => canRead('equ', null)(state))
	const canReadData = useAppSelector(state => canRead('data', null)(state))
	const canReadAdmin = useAppSelector(state => canRead('adm', null)(state))

	const listItems: IAppDrawerArray[] = useMemo(() => {
		const items: IAppDrawerArray[] = [{ id: 1, text: isMobile ? 'Main' : 'Main page', icon: <HomeIcon />, link: '/' }]
		if (canReadHouse) items.push({ id: 2, text: 'Houses', icon: <HouseIcon />, link: '/house' })
		if (canReadEquipment) items.push({ id: 3, text: 'Equipment', icon: <DeviceThermostatIcon />, link: '/equipment' })
		if (canReadData) items.push({ id: 4, text: 'Data', icon: <StorageIcon />, link: '/data' })
		return items
	}, [canReadHouse, canReadEquipment, canReadData, isMobile])

	const additionalItems: IAppDrawerArray[] = useMemo(() => {
		const items: IAppDrawerArray[] = []
		if (canReadAdmin) {
			items.push({
				id: 10,
				text: isMobile ? 'Admin' : 'Administration panel',
				icon: <AdminPanelSettingsIcon />,
				link: '/admin-panel',
			})
		}
		items.push({ id: 11, text: 'Profile', icon: <AccountBoxIcon />, link: '/profile' })
		items.push({ id: 12, text: 'Logout', icon: <LogoutIcon />, link: '' })
		return items
	}, [canReadAdmin, isMobile])

	return (
		<Box
			sx={{ width: isMobile ? 200 : 280, height: '100%', display: 'flex', flexDirection: 'column' }}
			role='presentation'>
			<Box
				sx={{
					p: 2.5,
					pb: 1.5,
					background: 'linear-gradient(135deg, #4a9e9e 0%, #357272 100%)',
					color: '#fff',
				}}>
				<Typography variant='h6' sx={{ fontWeight: 700, letterSpacing: '0.01em' }}>
					Logger Dashboard
				</Typography>
				<Typography variant='caption' sx={{ opacity: 0.8 }}>
					Navigation
				</Typography>
			</Box>
			<List sx={{ flexGrow: 1, pt: 1 }} onClick={() => toggleDrawer(false)}>
				{listItems.map(ele => (
					<AppDrawerItem key={ele.id} text={ele.text} icon={ele.icon} link={ele.link} />
				))}
			</List>
			<Divider sx={{ mx: 1.5 }} />
			<List sx={{ pb: 2, pt: 1 }} onClick={() => toggleDrawer(false)}>
				{additionalItems.map(ele => (
					<AppDrawerItem key={ele.id} text={ele.text} icon={ele.icon} link={ele.link} />
				))}
			</List>
		</Box>
	)
}
