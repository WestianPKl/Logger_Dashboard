import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'
import Equipment from '../equipment/equipment.model.js'
import HouseFloors from './houseFloors.model.js'

const HouseLogger = sequelize.define(
	'house_logger',
	{
		id: {
			type: Sequelize.INTEGER,
			autoIncrement: true,
			allowNull: false,
			primaryKey: true,
		},
		equLoggerId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'equ_logger_id',
		},
		houseFloorId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'house_floor_id',
		},
		posX: {
			type: Sequelize.INTEGER,
			allowNull: true,
			field: 'pos_x',
		},
		posY: {
			type: Sequelize.INTEGER,
			allowNull: true,
			field: 'pos_y',
		},
	},
	{
		timestamps: false,
		tableName: 'house_logger',
	}
)

HouseLogger.belongsTo(HouseFloors, {
	as: 'floor',
	targetKey: 'id',
	foreignKey: 'houseFloorId',
})
HouseFloors.hasMany(HouseLogger, {
	as: 'loggers',
	targetKey: 'id',
	foreignKey: 'houseFloorId',
})

HouseLogger.belongsTo(Equipment, {
	as: 'logger',
	targetKey: 'id',
	foreignKey: 'equLoggerId',
})
Equipment.hasMany(HouseLogger, {
	as: 'houseLogger',
	targetKey: 'id',
	foreignKey: 'equLoggerId',
})

export default HouseLogger
