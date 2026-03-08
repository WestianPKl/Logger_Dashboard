import express from 'express'
import path from 'path'
import { fileURLToPath } from 'url'
import cookieParser from 'cookie-parser'
import { notFound, internalServerError } from './util/responseHelper.js'
import cors from 'cors'
import fs from 'fs'
import morgan from 'morgan'
import compression from 'compression'
import equRouter from './api/routes/equipment.js'
import houseRouter from './api/routes/house.js'
import dataRouter from './api/routes/data.js'
import userRouter from './api/routes/users.js'
import admRouter from './api/routes/adm.js'
import processRouter from './api/routes/process.js'
import commonRouter from './api/routes/common.js'
import mqttRouter from './api/routes/mqtt.js'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

const app = express()

app.use(
	cors({
		origin: process.env.FRONTEND_ORIGIN?.split(',') || [
			'http://localhost:5173',
			'http://192.168.18.6:5173',
			'http://192.168.18.6:3000',
			'http://192.168.18.75:8080/',
		],
		credentials: true,
	})
)

app.use(compression())

const accessLogStream = fs.createWriteStream(
	path.join(__dirname, 'access.log'),
	{ flags: 'a' }
)
if (process.env.NODE_ENV !== 'production') {
	app.use(morgan('dev'))
}
app.use(morgan('combined', { stream: accessLogStream }))

app.use(express.json({ limit: '10mb' }))
app.use(express.urlencoded({ extended: false, limit: '10mb' }))
app.use(cookieParser())

app.use('/', express.static(path.join(__dirname, '../public')))
app.use('/uploads', express.static(path.join(__dirname, '../uploads')))

app.use('/api/user', userRouter)
app.use('/api/data', dataRouter)
app.use('/api/equipment', equRouter)
app.use('/api/process', processRouter)
app.use('/api/house', houseRouter)
app.use('/api/common', commonRouter)
app.use('/api/mqtt', mqttRouter)
app.use('/api/adm', admRouter)

if (process.env.NODE_ENV === 'production') {
	app.use(express.static(path.join(__dirname, '../dist-frontend')))
	app.get(/(.*)/, (req, res) => {
		res.sendFile(path.join(__dirname, '../dist-frontend', 'index.html'))
	})
}

app.use((req, res) => {
	return notFound(res, 'Not found')
})

app.use((err, req, res) => {
	return internalServerError(res, 'Error has occured.', err)
})

export default app
