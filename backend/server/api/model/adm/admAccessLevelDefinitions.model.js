import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const AccessLevelDefinition = sequelize.define(
	'adm_functionality_definition',
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
		accessLevel: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'access_level',
		},
	},
	{
		timestamps: false,
		tableName: 'adm_access_level_definition',
	}
)

export default AccessLevelDefinition
