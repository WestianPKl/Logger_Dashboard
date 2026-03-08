import { Sequelize } from 'sequelize'
import bcrypt from 'bcryptjs'
import sequelize from '../../util/database.js'
import {
	internalServerError,
	notFound,
	badRequest,
	serviceUnavailable,
	success,
	unauthorized,
	wrongValidation,
} from '../../util/responseHelper.js'
import { decodeSequelizeQuery } from '../../util/sequelizeTools.js'
import { checkPermission } from './permission.controller.js'
import { validationResult } from 'express-validator'
import { generateAccessToken } from '../../libs/jwtToken.js'
import SuperUsers from '../model/users/superusers.model.js'
import User from '../model/users/user.model.js'
import AdmPermissions from '../model/adm/admPermissions.js'
import { sendEmail } from '../../util/nodemailer.js'
import { deleteFile } from '../../middleware/file.js'
import AdmRoles from '../model/adm/admRoles.model.js'
import AdmRolesUser from '../model/adm/admRolesUser.model.js'
import crypto from 'crypto'
import path from 'path'
import sharp from 'sharp'

const Op = Sequelize.Op

export async function getUsers(req, res) {
	try {
		let permissionGranded = await checkPermission(
			req,
			'common',
			null,
			'READ'
		)
		if (!permissionGranded) {
			return unauthorized(res)
		}
		let queryObject = decodeSequelizeQuery(req.body)
		const data = await User.findAll({
			where: queryObject,
			attributes: { exclude: ['password'] },
		})
		if (!data) {
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function getUser(req, res) {
	try {
		const userId = req.params.userId
		const data = await User.findByPk(userId, {
			attributes: { exclude: ['password'] },
		})
		if (!data) {
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		return success(res, 'Data retrieved successfully', data)
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function registerUser(req, res) {
	const t = await sequelize.transaction()
	try {
		const errors = validationResult(req)
		if (!errors.isEmpty()) {
			return wrongValidation(res, 'Validation failed.', errors.array())
		}
		const hashedPassword = await bcrypt.hash(req.body.password, 8)
		let queryObject = {
			...req.body,
			password: hashedPassword,
		}
		const data = await User.create(queryObject, { transaction: t })
		const permissionRole = await AdmRoles.findOne({
			where: { name: 'Common' },
		})
		if (!permissionRole) {
			await t.rollback()
			return serviceUnavailable(res, 'Default role not found.')
		}
		const newUserPermission = await AdmRolesUser.create(
			{ roleId: permissionRole.id, userId: data.id },
			{ transaction: t }
		)
		if (!data || !newUserPermission) {
			await t.rollback()
			return serviceUnavailable(
				res,
				'Creating user or permissions failed.'
			)
		}
		await t.commit()
		return success(res, 'User registered successfully', data)
	} catch (err) {
		console.log(err)
		if (t) {
			await t.rollback()
		}
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function loginUser(req, res) {
	try {
		const errors = validationResult(req)
		if (!errors.isEmpty()) {
			return wrongValidation(res, 'Validation failed.', errors.array())
		}
		const username = req.body.username
		const password = req.body.password
		const user = await User.findOne({ where: { username: username } })
		if (!user) {
			return notFound(res, 'No such user.')
		}
		const doMatch = await bcrypt.compare(password, user.password)
		if (!doMatch) {
			return unauthorized(res, 'Wrong password.')
		}
		const cleanUser = {
			id: user.id,
			username: user.username,
			email: user.email,
			avatar: user.avatar,
			createdAt: user.createdAt,
			updatedAt: user.updatedAt,
		}

		const token = generateAccessToken({
			tokenType: 0,
			user: cleanUser,
		})
		const [userPermissions, superuser] = await Promise.all([
			AdmPermissions.findAll({ where: { userId: user.id } }),
			SuperUsers.findOne({ where: { userId: user.id } }),
		])
		if (!userPermissions) {
			return serviceUnavailable(res, 'No user permissions.')
		}
		let isSuperuser = false
		if (superuser) {
			isSuperuser = true
		}
		const expiration = new Date()
		expiration.setHours(expiration.getHours() + 1)
		const permissionToken = generateAccessToken({
			tokenType: 2,
			user: cleanUser,
			permissions: userPermissions,
			expiration: expiration,
			superuser: isSuperuser,
		})
		return success(res, 'Login successful', {
			token,
			permissionToken,
		})
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function updateUserProfile(req, res) {
	const t = await sequelize.transaction()
	try {
		const errors = validationResult(req)
		if (!errors.isEmpty()) {
			return wrongValidation(res, 'Validation failed.', errors.array())
		}
		const userId = req.params.userId
		const user = await User.findByPk(userId)
		if (!user) {
			return serviceUnavailable(res, 'No such user.')
		}

		const username = req.body.username
		const email = req.body.email

		const userNameCheck = await User.findOne({ where: { username } })
		if (userNameCheck && userNameCheck.id !== user.id) {
			return badRequest(res, 'Username exists already!')
		}
		const userEmailCheck = await User.findOne({ where: { email } })
		if (userEmailCheck && userEmailCheck.id !== user.id) {
			return badRequest(res, 'Email exists already!')
		}
		const password = req.body.password

		user.username = username
		user.email = email

		if (req.file !== undefined) {
			if (user.avatar) {
				try {
					deleteFile(user.avatar)
				} catch (err) {
					console.log('Avatar (thumb) delete error:', err)
				}
			}
			if (user.avatarBig) {
				try {
					deleteFile(user.avatarBig)
				} catch (err) {
					console.log('AvatarBig delete error:', err)
				}
			}
			user.avatarBig = req.file.path
			const ext = path.extname(req.file.filename)
			const thumbPath = req.file.path.replace(ext, `.thumb${ext}`)
			await sharp(req.file.path)
				.resize({
					width: 200,
					height: 200,
					fit: 'inside',
					withoutEnlargement: true,
				})
				.toFile(thumbPath)

			user.avatar = thumbPath
		}

		if (password) {
			const hashedPassword = await bcrypt.hash(password, 8)
			user.password = hashedPassword
		}

		const data = await user.save({ transaction: t })
		if (!data) {
			await t.rollback()
			return serviceUnavailable(res, 'Updating user failed.')
		}
		await t.commit()
		const userWithoutPassword = { ...data.toJSON() }
		delete userWithoutPassword.password
		return success(res, 'User updated successfully', userWithoutPassword)
	} catch (err) {
		console.log(err)
		if (t) {
			await t.rollback()
		}
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function passwordResetLink(req, res) {
	const t = await sequelize.transaction()
	try {
		const errors = validationResult(req)
		if (!errors.isEmpty()) {
			return wrongValidation(res, 'Validation failed.', errors.array())
		}
		const email = req.body.email
		const user = await User.findOne({
			where: { email: email },
		})
		if (!user) {
			return notFound(res, 'No such user.')
		}
		const token = crypto.randomBytes(32).toString('hex')
		const expires = Date.now() + 3600 * 1000

		user.resetPasswordToken = token
		user.resetPasswordExpires = new Date(expires)
		let resetLink = `${process.env.FRONTEND_URL_DEV}/password-reset/${token}`
		if (process.env.NODE_ENV === 'production') {
			resetLink = `${process.env.FRONTEND_URL_PROD}/password-reset/${token}`
		}
		const data = await user.save({ transaction: t })
		if (!data) {
			await t.rollback()
			return serviceUnavailable(res, 'Updating user failed.')
		}
		await t.commit()
		if (process.env.NODE_ENV === 'production') {
			sendEmail(
				email,
				'Reset your password',
				'',
				`<p>Click the link below to reset your password:</p>
            <a href="${resetLink}">${resetLink}</a>
            <p>Link valid for 1 hour.</p>`
			)
		} else {
			console.log(`Email sent to: ${email}`)
		}
		return success(res, 'Password reset link sent', {
			email: user.email,
			token,
		})
	} catch (err) {
		console.log(err)
		if (t) {
			await t.rollback()
		}
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function passwordReset(req, res) {
	const t = await sequelize.transaction()
	try {
		const errors = validationResult(req)
		if (!errors.isEmpty()) {
			return wrongValidation(res, 'Validation failed.', errors.array())
		}
		const token = req.params.token
		const password = req.body.password
		const user = await User.findOne({
			where: {
				resetPasswordToken: token,
				resetPasswordExpires: { [Op.gt]: new Date() },
			},
		})
		if (!user) return badRequest(res, 'Token invalid or expired')
		const hashedPassword = await bcrypt.hash(password, 8)
		user.password = hashedPassword
		user.resetPasswordToken = null
		user.resetPasswordExpires = null
		const data = await user.save({ transaction: t })
		if (!data) {
			await t.rollback()
			return serviceUnavailable(res, 'Updating user failed.')
		}
		await t.commit()
		return success(res, 'Password updated', {
			email: user.email,
		})
	} catch (err) {
		console.log(err)
		if (t) {
			await t.rollback()
		}
		return internalServerError(res, 'Error has occured.', err)
	}
}

export async function deleteUser(req, res) {
	const t = await sequelize.transaction()
	try {
		let permissionGranted = await checkPermission(
			req,
			'adm',
			'admUsers',
			'DELETE'
		)
		if (!permissionGranted) return unauthorized(res)
		const { id } = req.body
		if (!id) {
			return badRequest(res, 'Deleting data failed - no ID.')
		}
		const user = await User.findByPk(id)
		const permissions = await AdmPermissions.destroy({
			where: { userId: id },
			transaction: t,
		})
		const admRoleUsers = await AdmRolesUser.destroy({
			where: { userId: id },
			transaction: t,
		})
		if (user && user.avatar) {
			try {
				deleteFile(user.avatar)
			} catch (err) {
				console.log(err)
			}
		}
		if (user && user.avatarBig) {
			try {
				deleteFile(user.avatarBig)
			} catch (err) {
				console.log(err)
			}
		}
		const data = await User.destroy({
			where: { id },
			transaction: t,
		})
		if (!data && permissions && admRoleUsers) {
			await t.rollback()
			return serviceUnavailable(res, 'Retrieving data failed.')
		}
		await t.commit()
		return success(res, 'Data deleted successfully', data)
	} catch (err) {
		console.log(err)
		if (t) await t.rollback()
		return internalServerError(res, 'Error has occured.', err)
	}
}
