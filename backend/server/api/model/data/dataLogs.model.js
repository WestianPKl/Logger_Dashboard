import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'
import Equipment from '../equipment/equipment.model.js'
import DataDefinitions from './dataDefinitions.model.js'

const DataLogs = sequelize.define(
	'data_logs',
	{
		id: {
			type: Sequelize.INTEGER,
			autoIncrement: true,
			allowNull: false,
			primaryKey: true,
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
		dataDefinitionId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'data_definition_id',
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
	},
	{
		timestamps: false,
		tableName: 'data_logs',
	}
)

DataLogs.belongsTo(Equipment, {
	as: 'logger',
	targetKey: 'id',
	foreignKey: 'equLoggerId',
})
DataLogs.belongsTo(Equipment, {
	as: 'sensor',
	targetKey: 'id',
	foreignKey: 'equSensorId',
})
Equipment.hasMany(DataLogs, {
	as: 'measurements',
	targetKey: 'id',
	foreignKey: 'equSensorId',
})
DataLogs.belongsTo(DataDefinitions, {
	as: 'definition',
	targetKey: 'id',
	foreignKey: 'dataDefinitionId',
})

export default DataLogs
