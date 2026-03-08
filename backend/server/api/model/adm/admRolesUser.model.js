import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const AdmRolesUser = sequelize.define(
	'adm_roles_user',
	{
		roleId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			primaryKey: true,
			field: 'role_id',
		},
		userId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			primaryKey: true,
			field: 'user_id',
		},
	},
	{
		timestamps: false,
		tableName: 'adm_roles_user',
	}
)

export default AdmRolesUser
