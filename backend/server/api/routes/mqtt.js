import express from 'express'
import { mqttDevices } from '../controller/mqtt.controller.js'

const router = express.Router()

router.post('/:deviceId/cmd', mqttDevices)

export default router
