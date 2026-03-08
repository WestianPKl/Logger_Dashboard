import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'
import User from '../users/user.model.js'
import AdmRolesUser from './admRolesUser.model.js'

const AdmRoles = sequelize.define(
	'adm_roles',
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
		description: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'description',
		},
		createdById: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'created_by_id',
		},
		updatedById: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'updated_by_id',
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
		tableName: 'adm_roles',
	}
)

AdmRoles.belongsToMany(User, {
	as: 'users',
	through: AdmRolesUser,
	sourceKey: 'id',
	foreignKey: 'roleId',
	otherKey: 'userId',
	targetKey: 'id',
})
AdmRolesUser.belongsTo(AdmRoles, {
	as: 'role',
	targetKey: 'id',
	foreignKey: 'roleId',
})

AdmRoles.belongsTo(User, {
	as: 'createdBy',
	targetKey: 'id',
	foreignKey: 'createdById',
})
AdmRoles.belongsTo(User, {
	as: 'updatedBy',
	targetKey: 'id',
	foreignKey: 'updatedById',
})

export default AdmRoles
