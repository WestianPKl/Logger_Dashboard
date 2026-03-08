import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const ProcessType = sequelize.define(
	'process_type',
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
		tableName: 'process_type',
	}
)

export default ProcessType
