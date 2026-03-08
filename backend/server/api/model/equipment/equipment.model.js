import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'
import User from '../users/user.model.js'
import EquType from './equType.model.js'
import EquVendor from './equVendor.model.js'
import EquModel from './equModel.model.js'
import EquSensorFunctions from './equSensorsFunctions.model.js'
import DataDefinitions from '../data/dataDefinitions.model.js'

const Equipment = sequelize.define(
	'equ_equipment',
	{
		id: {
			type: Sequelize.INTEGER,
			autoIncrement: true,
			allowNull: false,
			primaryKey: true,
		},
		serialNumber: {
			type: Sequelize.STRING,
			allowNull: false,
			field: 'serial_number',
		},
		equVendorId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'equ_vendor_id',
		},
		equModelId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'equ_model_id',
		},
		equTypeId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'equ_type_id',
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
		deletedAt: {
			type: Sequelize.DATE,
			allowNull: true,
			field: 'deleted_at',
		},
		isDeleted: {
			type: Sequelize.INTEGER,
			allowNull: true,
			field: 'is_deleted',
		},
	},
	{
		timestamps: true,
		underscored: true,
		paranoid: true,
		tableName: 'equ_equipment',
	}
)

Equipment.belongsTo(User, {
	as: 'createdBy',
	targetKey: 'id',
	foreignKey: 'createdById',
})
Equipment.belongsTo(User, {
	as: 'updatedBy',
	targetKey: 'id',
	foreignKey: 'updatedById',
})
Equipment.belongsTo(EquType, {
	as: 'type',
	targetKey: 'id',
	foreignKey: 'equTypeId',
})
Equipment.belongsTo(EquVendor, {
	as: 'vendor',
	targetKey: 'id',
	foreignKey: 'equVendorId',
})
Equipment.belongsTo(EquModel, {
	as: 'model',
	targetKey: 'id',
	foreignKey: 'equModelId',
})

Equipment.belongsToMany(DataDefinitions, {
	as: 'dataDefinitions',
	through: EquSensorFunctions,
	sourceKey: 'id',
	foreignKey: 'equSensorId',
	otherKey: 'dataDefinitionId',
	targetKey: 'id',
})

export default Equipment
