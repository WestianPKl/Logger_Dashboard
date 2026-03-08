import { verifyToken } from '../libs/jwtToken.js'
import { unauthorized } from '../util/responseHelper.js'

export default function validateToken(req, res, next) {
	const authHeader = req.get('Authorization')
	if (!authHeader || !authHeader.startsWith('Bearer ')) {
		return unauthorized(res, 'Unauthorized access - token missing.')
	}
	const token = authHeader.split(' ')[1]
	if (!token) {
		return unauthorized(res, 'Unauthorized access - token does not exist.')
	}
	try {
		const decoded = verifyToken(token)
		req.user = decoded
		next()
	} catch (err) {
		console.error(err)
		return unauthorized(res, 'Unauthorized access - token is not valid.')
	}
}
