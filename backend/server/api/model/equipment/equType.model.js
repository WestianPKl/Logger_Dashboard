import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const EquType = sequelize.define(
	'equ_type',
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
		tableName: 'equ_type',
	}
)

export default EquType
