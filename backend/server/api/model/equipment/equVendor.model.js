import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const EquVendor = sequelize.define(
	'equ_vendor',
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
		tableName: 'equ_vendor',
	}
)

export default EquVendor
