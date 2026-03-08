import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'
import User from '../users/user.model.js'

const House = sequelize.define(
	'house_house',
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
		postalCode: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'postal_code',
		},
		city: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'city',
		},
		street: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'street',
		},
		houseNumber: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'house_number',
		},
		pictureLink: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'picture_link',
		},
		pictureLinkBig: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'picture_link_big',
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
		tableName: 'house_house',
	}
)

House.belongsTo(User, {
	as: 'createdBy',
	targetKey: 'id',
	foreignKey: 'createdById',
})
House.belongsTo(User, {
	as: 'updatedBy',
	targetKey: 'id',
	foreignKey: 'updatedById',
})

export default House
