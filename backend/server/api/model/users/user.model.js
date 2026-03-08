import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'

const User = sequelize.define(
	'users',
	{
		id: {
			type: Sequelize.INTEGER,
			autoIncrement: true,
			allowNull: false,
			primaryKey: true,
		},
		username: {
			type: Sequelize.STRING,
			allowNull: false,
			unique: true,
			field: 'username',
		},
		email: {
			type: Sequelize.STRING,
			allowNull: false,
			unique: true,
			field: 'email',
		},
		password: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'password',
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
		confirmed: {
			type: Sequelize.BOOLEAN,
			allowNull: true,
			defaultValue: false,
			field: 'confirmed',
		},
		avatar: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'avatar',
		},
		avatarBig: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'avatar_big',
		},
		resetPasswordToken: {
			type: Sequelize.STRING,
			allowNull: true,
			field: 'reset_password_token',
		},
		resetPasswordExpires: {
			type: Sequelize.DATE,
			allowNull: true,
			field: 'reset_password_expires',
		},
	},
	{
		timestamps: true,
		underscored: true,
		tableName: 'users',
	}
)

export default User
