import io from 'socket.io-client'

export const socket = io(import.meta.env.VITE_API_IP, {
	timeout: 5000,
	reconnectionAttempts: 3,
	reconnectionDelay: 1000,
})

socket.on('connect', () => {
	console.log('WebSocket connected')
})
socket.on('disconnect', () => {
	console.log('WebSocket disconnected')
})
socket.on('error', (err: any) => {
	console.error('WebSocket error:', err)
})
