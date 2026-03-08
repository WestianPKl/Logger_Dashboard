import express from 'express'
import {
	registerUser,
	loginUser,
	updateUserProfile,
	getUsers,
	getUser,
	deleteUser,
	passwordResetLink,
	passwordReset,
} from '../controller/users.controller.js'
import {
	usernameLogin,
	passwordLogin,
	usernameRegister,
	emailRegister,
	passwordRegister,
	confirmPasswordRegister,
	usernameReset,
	emailReset,
} from '../../middleware/body-validation.js'
import { imageUpload } from '../../middleware/file.js'
import validateToken from '../../middleware/jwtValidation.js'

const router = express.Router()

router.post('/users', validateToken, getUsers)
router.get('/user/:userId', validateToken, getUser)
router.post(
	'/user-register',
	[
		usernameRegister,
		emailRegister,
		passwordRegister,
		confirmPasswordRegister,
	],
	registerUser
)

router.post('/user-login', [usernameLogin, passwordLogin], loginUser)
router.patch(
	'/user/:userId',
	validateToken,
	imageUpload.single('avatar'),
	[usernameReset.optional(), emailReset.optional()],
	updateUserProfile
)
router.delete('/user/:userId', validateToken, deleteUser)
router.post('/reset-password-request', passwordResetLink)
router.post(
	'/reset-password/:token',
	[passwordRegister, confirmPasswordRegister],
	passwordReset
)

export default router
