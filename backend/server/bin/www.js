import app from '../app.js'
import http from 'http'
import 'dotenv/config'
import { initSocket } from '../middleware/socket.js'

const port = normalizePort(
	process.env.NODE_ENV === 'production'
		? process.env.PORT_PROD || '8000'
		: process.env.PORT_DEV || '3000'
)
app.set('port', port)

const server = http.createServer(app)
server.listen(port)
server.on('error', onError)
server.on('listening', () => {
	const addr = server.address()
	console.log(
		`Server listening on ${typeof addr === 'string' ? addr : addr?.port}`
	)
})

initSocket(server)

function normalizePort(val) {
	const port = parseInt(val, 10)
	if (isNaN(port)) return val
	if (port >= 0) return port
	return false
}

function onError(error) {
	if (error.syscall !== 'listen') throw error

	if (error.code === 'EACCES') {
		console.error(`${port} requires elevated privileges`)
		process.exit(1)
	}

	if (error.code === 'EADDRINUSE') {
		console.error(`${port} is already in use`)
		process.exit(1)
	}

	throw error
}
