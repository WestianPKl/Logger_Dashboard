import fs from 'fs'
import multer from 'multer'
import { v1 } from 'uuid'

const MIME_TYPE_MAP = {
	'image/png': 'png',
	'image/jpeg': 'jpeg',
	'image/jpg': 'jpg',
}

export function createFolder(path) {
	if (!fs.existsSync(path)) {
		fs.mkdirSync(path, { recursive: true })
	}
}

export const fileStorage = multer.diskStorage({
	destination: (req, file, cb) => {
		createFolder('uploads/')
		cb(null, 'uploads/')
	},
	filename: (req, file, cb) => {
		const ext = MIME_TYPE_MAP[file.mimetype]
		if (!ext) return cb(new Error('Unsupported file type'), null)
		cb(null, `${v1()}.${ext}`)
	},
})

function fileFilter(req, file, cb) {
	const isValid = !!MIME_TYPE_MAP[file.mimetype]
	cb(isValid ? null : new Error('Invalid mime type!'), isValid)
}

export const imageUpload = multer({
	limits: 500000,
	storage: fileStorage,
	fileFilter: fileFilter,
})

export function deleteFile(filePath) {
	if (!filePath) {
		console.error('deleteFile: filePath is undefined!')
		return
	}
	fs.unlink(filePath, (err) => {
		if (err) console.error('Could not delete file:', err)
	})
}
