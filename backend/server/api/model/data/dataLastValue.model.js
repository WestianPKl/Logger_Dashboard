import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'
import DataLogs from './dataLogs.model.js'
import Equipment from '../equipment/equipment.model.js'

const DataLastValue = sequelize.define(
	'data_last_value',
	{
		id: {
			type: Sequelize.INTEGER,
			autoIncrement: true,
			allowNull: false,
			primaryKey: true,
		},
		dataLogId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'data_log_id',
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
		createdAt: {
			type: Sequelize.DATE,
			allowNull: false,
			field: 'created_at',
		},
		updatedAt: {
			type: Sequelize.DATE,
			allowNull: false,
			field: 'updated_at',
		},
	},
	{
		timestamps: true,
		underscored: true,
		tableName: 'data_last_value',
	}
)

DataLastValue.belongsTo(Equipment, {
	as: 'logger',
	targetKey: 'id',
	foreignKey: 'equLoggerId',
})

Equipment.hasOne(DataLastValue, {
	as: 'lastValue',
	targetKey: 'id',
	foreignKey: 'equLoggerId',
})

DataLastValue.belongsTo(DataLogs, {
	as: 'log',
	targetKey: 'id',
	foreignKey: 'dataLogId',
})

export default DataLastValue
