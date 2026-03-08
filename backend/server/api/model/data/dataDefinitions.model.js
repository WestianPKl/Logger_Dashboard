import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const DataDefinitions = sequelize.define(
	'data_definitions',
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
		unit: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'unit',
		},
		description: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'description',
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
		tableName: 'data_definitions',
	}
)

export default DataDefinitions
