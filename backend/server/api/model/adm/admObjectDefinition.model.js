import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const ObjectDefinition = sequelize.define(
	'adm_object_definition',
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
	},
	{
		timestamps: false,
		tableName: 'adm_object_definition',
	}
)

export default ObjectDefinition
