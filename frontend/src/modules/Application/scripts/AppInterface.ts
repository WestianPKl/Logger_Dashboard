export interface IErrorData {
	location: string
	msg: string
	path: string
	type: string
	value: string
}

export interface IAppDrawerListProps {
	toggleDrawer: (state: boolean) => void
}

export interface IAppDrawerItemProps {
	text: string
	icon: React.ReactElement<unknown>
	link: string
}

export interface IAppNavigationItemProps {
	text: string
	link: string
}

export interface IAppDrawerArray {
	id: number
	text: string
	link: string
	icon: React.ReactElement<unknown>
}

export interface IAppNavigationArray {
	id: number
	text: string
	link: string
}
