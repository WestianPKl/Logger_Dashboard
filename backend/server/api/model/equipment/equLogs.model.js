import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'
import Equipment from './equipment.model.js'

const EquLogs = sequelize.define(
	'equ_log',
	{
		id: {
			type: Sequelize.INTEGER,
			autoIncrement: true,
			allowNull: false,
			primaryKey: true,
		},
		equipmentId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'equipment_id',
		},
		message: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'message',
		},
		type: {
			type: Sequelize.ENUM('STATUS', 'DATA', 'OTA', 'ERROR', 'OTHER'),
			allowNull: false,
			field: 'type',
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
		tableName: 'equ_log',
	}
)

EquLogs.belongsTo(Equipment, {
	as: 'equipment',
	targetKey: 'id',
	foreignKey: 'equipmentId',
})
Equipment.hasMany(EquLogs, {
	as: 'logs',
	targetKey: 'id',
	foreignKey: 'equipmentId',
})

export default EquLogs
