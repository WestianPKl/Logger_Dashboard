import { Server } from 'socket.io'

let io = null

export function initSocket(server) {
	io = new Server(server, {
		cors: {
			origin: process.env.FRONTEND_ORIGIN?.split(',') || [
				'http://localhost:5173',
				'http://192.168.18.6:5173',
				'http://192.168.18.6:3000',
				'http://192.168.18.75:8080',
			],
		},
	})
	return io
}

export function getIo() {
	if (!io) {
		if (process.env.NODE_ENV === 'test')
			return { sockets: { emit: () => {} } }
		throw new Error('Socket.io not initialized')
	}
	return io
}
