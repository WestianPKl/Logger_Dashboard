import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const SuperUsers = sequelize.define(
	'superusers',
	{
		id: {
			type: Sequelize.INTEGER,
			autoIncrement: true,
			allowNull: false,
			primaryKey: true,
		},
		userId: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'user_id',
		},
	},
	{
		timestamps: false,
		tableName: 'superusers',
	}
)

export default SuperUsers
