import { createTheme } from '@mui/material'

export const theme = createTheme({
	palette: {
		mode: 'light',
		primary: {
			main: '#4a9e9e',
			light: '#6ec4c4',
			dark: '#357272',
			contrastText: '#fff',
		},
		secondary: {
			main: '#607d8b',
			light: '#90a4ae',
			dark: '#455a64',
			contrastText: '#fff',
		},
		background: {
			default: '#f0f4f8',
			paper: '#ffffff',
		},
		text: {
			primary: '#1a2027',
			secondary: '#546e7a',
			disabled: 'rgba(0, 0, 0, 0.3)',
		},
		divider: 'rgba(0, 0, 0, 0.08)',
		error: {
			main: '#ef5350',
			light: '#ef9a9a',
			dark: '#c62828',
			contrastText: '#fff',
		},
		warning: {
			main: '#ffa726',
			light: '#ffcc80',
			dark: '#ef6c00',
			contrastText: 'rgba(0, 0, 0, 0.87)',
		},
		info: {
			main: '#42a5f5',
			light: '#90caf9',
			dark: '#1565c0',
			contrastText: '#fff',
		},
		success: {
			main: '#66bb6a',
			light: '#a5d6a7',
			dark: '#2e7d32',
			contrastText: '#fff',
		},
	},
	shape: {
		borderRadius: 12,
	},
	typography: {
		fontFamily: '"Inter", "Roboto", "Helvetica", "Arial", sans-serif',
		h5: {
			fontWeight: 600,
			letterSpacing: '-0.01em',
		},
		h6: {
			fontWeight: 600,
			letterSpacing: '-0.005em',
		},
		subtitle1: {
			fontWeight: 500,
		},
		button: {
			textTransform: 'none',
			fontWeight: 600,
		},
	},
	components: {
		MuiCssBaseline: {
			styleOverrides: {
				body: {
					backgroundImage: 'linear-gradient(135deg, #f0f4f8 0%, #e8eef5 100%)',
					minHeight: '100vh',
				},
			},
		},
		MuiAppBar: {
			styleOverrides: {
				root: {
					backgroundImage: 'linear-gradient(135deg, #4a9e9e 0%, #357272 100%)',
					boxShadow: '0 2px 12px rgba(74, 158, 158, 0.3)',
				},
			},
		},
		MuiButton: {
			styleOverrides: {
				root: {
					borderRadius: 8,
					padding: '8px 20px',
					transition: 'all 0.2s ease-in-out',
				},
				contained: {
					boxShadow: '0 2px 8px rgba(0, 0, 0, 0.12)',
					'&:hover': {
						boxShadow: '0 4px 16px rgba(0, 0, 0, 0.18)',
						transform: 'translateY(-1px)',
					},
				},
				outlined: {
					borderWidth: 1.5,
					'&:hover': {
						borderWidth: 1.5,
					},
				},
			},
		},
		MuiCard: {
			styleOverrides: {
				root: {
					borderRadius: 16,
					boxShadow: '0 2px 12px rgba(0, 0, 0, 0.06)',
					transition: 'box-shadow 0.3s ease, transform 0.3s ease',
					'&:hover': {
						boxShadow: '0 8px 30px rgba(0, 0, 0, 0.12)',
						transform: 'translateY(-4px)',
					},
				},
			},
		},
		MuiPaper: {
			styleOverrides: {
				root: {
					backgroundImage: 'none',
				},
				elevation1: {
					boxShadow: '0 1px 8px rgba(0, 0, 0, 0.06)',
				},
			},
		},
		MuiDrawer: {
			styleOverrides: {
				paper: {
					borderRight: 'none',
					boxShadow: '4px 0 24px rgba(0, 0, 0, 0.08)',
				},
			},
		},
		MuiTextField: {
			styleOverrides: {
				root: {
					'& .MuiOutlinedInput-root': {
						borderRadius: 10,
						transition: 'box-shadow 0.2s ease',
						'&.Mui-focused': {
							boxShadow: '0 0 0 3px rgba(74, 158, 158, 0.15)',
						},
					},
				},
			},
		},
		MuiDialog: {
			styleOverrides: {
				paper: {
					borderRadius: 16,
					boxShadow: '0 12px 40px rgba(0, 0, 0, 0.15)',
				},
			},
		},
		MuiTab: {
			styleOverrides: {
				root: {
					textTransform: 'none',
					fontWeight: 500,
					fontSize: '0.9rem',
					minHeight: 48,
				},
			},
		},
		MuiTabs: {
			styleOverrides: {
				indicator: {
					height: 3,
					borderRadius: '3px 3px 0 0',
				},
			},
		},
		MuiAlert: {
			styleOverrides: {
				root: {
					borderRadius: 12,
				},
				filled: {
					boxShadow: '0 4px 16px rgba(0, 0, 0, 0.15)',
				},
			},
		},
		MuiChip: {
			styleOverrides: {
				root: {
					fontWeight: 500,
				},
			},
		},
		MuiTooltip: {
			styleOverrides: {
				tooltip: {
					borderRadius: 8,
					fontSize: '0.8rem',
				},
			},
		},
		MuiListItemButton: {
			styleOverrides: {
				root: {
					borderRadius: 8,
					margin: '2px 8px',
					'&:hover': {
						backgroundColor: 'rgba(74, 158, 158, 0.08)',
					},
				},
			},
		},
		MuiAvatar: {
			styleOverrides: {
				root: {
					boxShadow: '0 2px 8px rgba(0, 0, 0, 0.12)',
				},
			},
		},
	},
})
