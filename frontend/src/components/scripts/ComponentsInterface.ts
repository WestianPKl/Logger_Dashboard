export interface ISnackBarProps {
	message: string[] | string
	severity: 'success' | 'info' | 'warning' | 'error'
}

export interface IPropsComponentsWrapper {
	children: React.ReactNode
}

const formatLocalDateTime = (date: Date | number | string): string => {
	const d = new Date(date)
	const year = d.getFullYear()
	const month = String(d.getMonth() + 1).padStart(2, '0')
	const day = String(d.getDate()).padStart(2, '0')
	const hours = String(d.getHours()).padStart(2, '0')
	const minutes = String(d.getMinutes()).padStart(2, '0')
	const seconds = String(d.getSeconds()).padStart(2, '0')
	return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`
}
export default formatLocalDateTime
