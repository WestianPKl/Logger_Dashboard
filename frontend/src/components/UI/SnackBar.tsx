import { Alert, Box } from '@mui/material'
import type { ISnackBarProps } from '../scripts/ComponentsInterface'

export default function SnackBar({ message, severity }: ISnackBarProps) {
	let alertElement

	const alertSx = {
		position: 'fixed' as const,
		bottom: 24,
		left: '50%',
		transform: 'translateX(-50%)',
		minWidth: 320,
		maxWidth: '90vw',
		zIndex: 1400,
		animation: 'slideUp 0.3s ease-out',
		'@keyframes slideUp': {
			from: { opacity: 0, transform: 'translateX(-50%) translateY(20px)' },
			to: { opacity: 1, transform: 'translateX(-50%) translateY(0)' },
		},
	}

	if (Array.isArray(message) && message.length >= 1) {
		alertElement = (
			<Alert variant='filled' sx={alertSx} severity={severity}>
				<Box sx={{ display: 'flex', flexDirection: 'column', gap: 0.5 }}>
					{message.map((e: string, index: number) => (
						<span key={index}>{e}</span>
					))}
				</Box>
			</Alert>
		)
	} else if (typeof message === 'string') {
		alertElement = (
			<Alert variant='filled' sx={alertSx} severity={severity}>
				{message}
			</Alert>
		)
	}

	return <>{alertElement}</>
}
