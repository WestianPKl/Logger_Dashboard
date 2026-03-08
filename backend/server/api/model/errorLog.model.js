import { Sequelize } from 'sequelize'
import sequelize from '../../util/database.js'
import Equipment from './equipment/equipment.model.js'

const ErrorLog = sequelize.define(
	'error_log',
	{
		id: {
			type: Sequelize.INTEGER,
			autoIncrement: true,
			allowNull: false,
			primaryKey: true,
		},
		message: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'message',
		},
		details: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'details',
		},
		type: {
			type: Sequelize.ENUM(['Equipment', 'DB', 'Other']),
			allowNull: false,
			field: 'type',
		},
		severity: {
			type: Sequelize.ENUM(['Critical', 'Error', 'Warning', 'Info']),
			allowNull: false,
			field: 'severity',
		},
		equipmentId: {
			type: Sequelize.INTEGER,
			allowNull: true,
			field: 'equipment_id',
		},
		createdAt: {
			type: Sequelize.DATE,
			allowNull: true,
			field: 'created_at',
		},
		updatedAt: {
			type: Sequelize.DATE,
			allowNull: true,
			field: 'updated_at',
		},
	},
	{
		timestamps: true,
		underscored: true,
		tableName: 'error_log',
	}
)

ErrorLog.belongsTo(Equipment, {
	as: 'equipment',
	targetKey: 'id',
	foreignKey: 'equipmentId',
})
Equipment.hasMany(ErrorLog, {
	as: 'errors',
	targetKey: 'id',
	foreignKey: 'equipmentId',
})

export default ErrorLog
