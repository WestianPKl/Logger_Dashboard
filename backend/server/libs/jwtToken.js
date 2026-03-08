import jwt from 'jsonwebtoken'
import 'dotenv/config'

const TOKEN_SECRET = process.env.TOKEN
const TOKEN_SECRET_PERMISSION = process.env.TOKEN_SECRET_PERMISSION
const TOKEN_EXPIRE_TIME = '5h'

export function generateAccessToken(data) {
	let token = null
	switch (data.tokenType) {
		case 0:
			token = jwt.sign(data, TOKEN_SECRET, {
				expiresIn: TOKEN_EXPIRE_TIME,
			})
			break
		case 1:
			token = jwt.sign(data, TOKEN_SECRET, { expiresIn: '365d' }) // 1 rok
			break
		case 2:
			token = jwt.sign(data, TOKEN_SECRET_PERMISSION, {
				expiresIn: TOKEN_EXPIRE_TIME,
			})
			break
		case 3:
			token = jwt.sign(data, TOKEN_SECRET, { expiresIn: '1m' }) // 1 minuta
			break
		default:
			throw new Error('Unknown token type')
	}
	return token
}

export function verifyToken(token, secret = TOKEN_SECRET) {
	try {
		return jwt.verify(token, secret)
	} catch (err) {
		console.error(err)
		return false
	}
}

export function getUserDetail(req, type = 'default') {
	let authHeader = req.get('Authorization')
	let token = authHeader && authHeader.split(' ')[1]
	if (!token) {
		return null
	}
	let secret = type === 'permission' ? TOKEN_SECRET_PERMISSION : TOKEN_SECRET
	let tokenDecode = verifyToken(token, secret)
	return tokenDecode && tokenDecode.user ? tokenDecode.user : null
}

export function getUserId(req) {
	const user = getUserDetail(req)
	return user ? user.id : null
}

export function getUserPermission(req) {
	const user = getUserDetail(req)
	return user ? user.permission : null
}

export function getUserEmail(req) {
	const user = getUserDetail(req)
	return user ? user.email : null
}
