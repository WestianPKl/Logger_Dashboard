import { internalServerError } from '../../util/responseHelper.js'
import { mqttClient } from '../../mqttClient.js'

export async function mqttDevices(req, res) {
	try {
		const deviceId = req.params.deviceId

		const command = req.body.command || 'UNKNOWN'
		const parameters = req.body.parameters || {}

		const topic = `devices/${deviceId}/cmd`
		const payload = JSON.stringify({
			cmd: command,
			params: parameters,
			ts: Date.now(),
		})

		mqttClient.publish(topic, payload, { qos: 1, retain: false }, (err) => {
			if (err) {
				return res.status(500).json({ ok: false, error: err.message })
			}
			return res.json({ ok: true, topic, payload: JSON.parse(payload) })
		})
	} catch (err) {
		console.log(err)
		return internalServerError(res, 'Error has occured.', err)
	}
}
