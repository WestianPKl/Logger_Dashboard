import { Sequelize } from 'sequelize'
import 'dotenv/config'

let DB = process.env.DB_DEV || ''
let DB_USER = process.env.DBUSER_DEV || ''
let DB_PASSWORD = process.env.DBPASSWORD_DEV || ''
let DB_HOST = process.env.DBHOST_DEV || 'localhost'
let DB_PORT = process.env.DBPORT_DEV || 3306

if (process.env.NODE_ENV === 'production') {
	DB = process.env.DB_PROD || ''
	DB_USER = process.env.DBUSER_PROD || ''
	DB_PASSWORD = process.env.DBPASSWORD_PROD || ''
	DB_HOST = process.env.DBHOST_PROD || 'localhost'
	DB_PORT = process.env.DBPORT_PROD || 3306
}

const sequelize = new Sequelize(DB, DB_USER, DB_PASSWORD, {
	host: DB_HOST,
	port: DB_PORT,
	dialect: 'mysql',
	pool: {
		max: 90,
		min: 0,
		idle: 10000,
	},
	logging: false,
	dialectOptions: {
		timezone: 'Z',
	},
})

export default sequelize
