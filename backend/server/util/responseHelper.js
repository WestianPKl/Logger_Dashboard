export function internalServerError(
	res,
	msg = 'Internal server error.',
	data = {}
) {
	return res.status(500).json({
		success: false,
		message: msg,
		...(data && Object.keys(data).length > 0 ? { data } : {}),
	})
}

export function unauthorized(res, msg = 'Unauthorized.', data = {}) {
	return res.status(401).json({
		success: false,
		message: msg,
		...(data && Object.keys(data).length > 0 ? { data } : {}),
	})
}

export function notFound(res, msg = 'Not found.', data = {}) {
	return res.status(404).json({
		success: false,
		message: msg,
		...(data && Object.keys(data).length > 0 ? { data } : {}),
	})
}

export function badRequest(res, msg = 'Bad request.', data = {}) {
	return res.status(400).json({
		success: false,
		message: msg,
		...(data && Object.keys(data).length > 0 ? { data } : {}),
	})
}

export function wrongValidation(res, msg = 'Wrong validation.', data = {}) {
	return res.status(422).json({
		success: false,
		message: msg,
		...(data && Object.keys(data).length > 0 ? { data } : {}),
	})
}

export function serviceUnavailable(
	res,
	msg = 'Service unavailable.',
	data = {}
) {
	return res.status(503).json({
		success: false,
		message: msg,
		...(data && Object.keys(data).length > 0 ? { data } : {}),
	})
}

export function success(res, msg = 'Success.', data) {
	return res.status(200).json({
		success: true,
		message: msg,
		...(data && Object.keys(data).length > 0 ? { data } : {}),
	})
}

export function created(res, msg = 'Created.', data) {
	return res.status(201).json({
		success: true,
		message: msg,
		...(data && Object.keys(data).length > 0 ? { data } : {}),
	})
}

export function noContent(res) {
	return res.status(204).send()
}

export function forbidden(res, msg = 'Forbidden.', data = {}) {
	return res.status(403).json({
		success: false,
		message: msg,
		...(data && Object.keys(data).length > 0 ? { data } : {}),
	})
}

export function conflict(res, msg = 'Conflict.', data = {}) {
	return res.status(409).json({
		success: false,
		message: msg,
		...(data && Object.keys(data).length > 0 ? { data } : {}),
	})
}

export function tooManyRequests(res, msg = 'Too many requests.', data = {}) {
	return res.status(429).json({
		success: false,
		message: msg,
		...(data && Object.keys(data).length > 0 ? { data } : {}),
	})
}
