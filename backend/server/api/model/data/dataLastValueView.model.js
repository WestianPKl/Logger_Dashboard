import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const DataLastValueView = sequelize.define(
	'data_view_last_value',
	{
		id: {
			type: Sequelize.INTEGER,
			allowNull: false,
			primaryKey: true,
		},
		equLoggerId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'equ_logger_id',
		},
		equSensorId: {
			type: Sequelize.INTEGER,
			allowNull: false,
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
		time: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'time',
		},
		value: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'value',
		},
		parameter: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'parameter',
		},
		unit: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'unit',
		},
	},
	{
		timestamps: false,
		tableName: 'data_view_last_value',
	}
)

export default DataLastValueView
