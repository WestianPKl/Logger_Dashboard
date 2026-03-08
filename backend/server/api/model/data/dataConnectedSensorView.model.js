import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const DataConnectedSensorView = sequelize.define(
	'data_view_connected_sensor',
	{
		equLoggerId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'equ_logger_id',
		},
		equSensorId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			primaryKey: true,
			field: 'equ_sensor_id',
		},
		houseFloorId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'house_floor_id',
		},
		houseLoggerId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'house_logger_id',
		},
		sensorVendor: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'sensor_vendor',
		},
		sensorModel: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'sensor_model',
		},
		sensorSerialNumber: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'sensor_serial_number',
		},
	},
	{
		timestamps: false,
		tableName: 'data_view_connected_sensor',
	}
)

export default DataConnectedSensorView
