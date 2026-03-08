import { lazy, Suspense } from 'react'
import { useNavigate } from 'react-router'
import { AppBar, Box, Toolbar, Typography, Button, CircularProgress, useMediaQuery, useTheme } from '@mui/material'
import { selectIsLogged } from '../../store/account-store'
import { useAppSelector } from '../../store/hooks'
const AppDrawer = lazy(() => import('./components/AppDrawer'))
const AppMenu = lazy(() => import('./components/AppMenu'))

export default function AppBarView() {
	const isLogged = useAppSelector(selectIsLogged)
	const navigate = useNavigate()

	const theme = useTheme()
	const isMobile = useMediaQuery(theme.breakpoints.down('sm'))

	function loginButtonHandler(): void {
		navigate('/login')
	}

	function registerButtonHandler(): void {
		navigate('/register')
	}

	return (
		<Box sx={{ flexGrow: 1 }}>
			<AppBar position='sticky' elevation={0}>
				<Toolbar sx={{ minHeight: { xs: 56, sm: 64 } }}>
					{isLogged ? (
						<Suspense
							fallback={
								<Box sx={{ display: 'flex', alignItems: 'center', flexGrow: 1, justifyContent: 'center' }}>
									<CircularProgress size={24} sx={{ color: 'inherit' }} />
								</Box>
							}>
							<AppDrawer />
							<Typography
								variant='h6'
								component='div'
								sx={{
									flexGrow: 1,
									letterSpacing: '0.02em',
									fontWeight: 700,
								}}>
								Logger Dashboard
							</Typography>
							<AppMenu />
						</Suspense>
					) : (
						<Box sx={{ display: 'flex', gap: 1, width: '100%', justifyContent: 'flex-end' }}>
							<Typography variant='h6' component='div' sx={{ flexGrow: 1, fontWeight: 700, letterSpacing: '0.02em' }}>
								Logger Dashboard
							</Typography>
							<Button
								color='inherit'
								size={isMobile ? 'small' : 'medium'}
								variant='text'
								sx={{ fontWeight: 500 }}
								onClick={loginButtonHandler}>
								Login
							</Button>
							<Button
								color='inherit'
								size={isMobile ? 'small' : 'medium'}
								variant='outlined'
								sx={{
									borderColor: 'rgba(255,255,255,0.5)',
									'&:hover': { borderColor: '#fff', bgcolor: 'rgba(255,255,255,0.1)' },
								}}
								onClick={registerButtonHandler}>
								Register
							</Button>
						</Box>
					)}
				</Toolbar>
			</AppBar>
		</Box>
	)
}
