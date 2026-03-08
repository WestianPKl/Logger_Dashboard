import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const EquSensorFunctions = sequelize.define(
	'equ_sensor_functions',
	{
		equSensorId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			primaryKey: true,
			field: 'equ_sensor_id',
		},
		dataDefinitionId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			primaryKey: true,
			field: 'data_definition_id',
		},
	},
	{
		timestamps: false,
		tableName: 'equ_sensor_functions',
	}
)

export default EquSensorFunctions
