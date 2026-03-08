export interface IToken {
	token: string
	permissionToken: string
}

export interface ILoginData {
	username: string
	password: string
}

export interface IRegisterData {
	username: string
	email: string
	password: string
	confirmPassword: string
}

export interface ILoginFormProps {
	logIn: (loginData: ILoginData) => void
}

export interface IPasswordResetLinkFormProps {
	getPasswordResetLink: (email: string | undefined) => void
}

export interface IPasswordResetFormProps {
	getPasswordReset: (password: string | undefined, confirmPassword: string | undefined) => void
}

export interface IRegisterFormProps {
	createNewAccount: (registerData: IRegisterData) => void
}
