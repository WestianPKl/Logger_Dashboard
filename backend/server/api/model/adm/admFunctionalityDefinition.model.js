import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const FunctionalityDefinition = sequelize.define(
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
		description: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'description',
		},
	},
	{
		timestamps: false,
		tableName: 'adm_functionality_definition',
	}
)

export default FunctionalityDefinition
