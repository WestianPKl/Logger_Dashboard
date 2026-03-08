import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'
import Equipment from './equipment.model.js'

const EquStats = sequelize.define(
	'equ_stats',
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
		lastSeen: {
			type: Sequelize.DATE,
			allowNull: false,
			field: 'last_seen',
		},
		snContr: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'sn_contr',
		},
		fwContr: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'fw_contr',
		},
		hwContr: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'hw_contr',
		},
		buildContr: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'build_contr',
		},
		prodContr: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'prod_contr',
		},
		snCom: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'sn_com',
		},
		fwCom: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'fw_com',
		},
		hwCom: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'hw_com',
		},
		buildCom: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'build_com',
		},
		prodCom: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'prod_com',
		},
		ipAddress: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'ip_address',
		},
	},
	{
		timestamps: false,
		tableName: 'equ_stats',
	}
)

EquStats.belongsTo(Equipment, {
	as: 'equipment',
	targetKey: 'id',
	foreignKey: 'equipmentId',
})
Equipment.hasOne(EquStats, {
	as: 'stats',
	targetKey: 'id',
	foreignKey: 'equipmentId',
})

export default EquStats
