import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const EquModel = sequelize.define(
	'equ_model',
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
	},
	{
		timestamps: false,
		tableName: 'equ_model',
	}
)

export default EquModel
