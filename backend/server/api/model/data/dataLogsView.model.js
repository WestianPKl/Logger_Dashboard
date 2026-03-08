import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const DataLogsView = sequelize.define(
	'data_view_logs',
	{
		equLoggerId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			primaryKey: true,
			field: 'equ_logger_id',
		},
		time: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'time',
		},
		equSensorId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'equ_sensor_id',
		},
		temperature: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'temperature',
		},
		humidity: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'humidity',
		},
		atmPressure: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'atmPressure',
		},
		altitude: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'altitude',
		},
	},
	{
		timestamps: false,
		tableName: 'data_view_logs',
	}
)

export default DataLogsView
