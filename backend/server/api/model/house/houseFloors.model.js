import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'
import House from './house.model.js'

const HouseFloors = sequelize.define(
	'house_floors',
	{
		id: {
			type: Sequelize.INTEGER,
			autoIncrement: true,
			allowNull: false,
			primaryKey: true,
		},
		name: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'name',
		},
		layout: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'layout',
		},
		layoutBig: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'layout_big',
		},
		houseId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'house_id',
		},
		x: {
			type: Sequelize.INTEGER,
			allowNull: true,
			field: 'x',
		},
		y: {
			type: Sequelize.INTEGER,
			allowNull: true,
			field: 'y',
		},
		zoom: {
			type: Sequelize.INTEGER,
			allowNull: true,
			field: 'zoom',
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
		tableName: 'house_floors',
	}
)

HouseFloors.belongsTo(House, {
	as: 'house',
	targetKey: 'id',
	foreignKey: 'houseId',
})

House.hasMany(HouseFloors, {
	as: 'floors',
	targetKey: 'id',
	foreignKey: 'houseId',
})

export default HouseFloors
