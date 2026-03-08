import { Sequelize } from 'sequelize'
import sequelize from '../../../util/database.js'
import FunctionalityDefinition from './admFunctionalityDefinition.model.js'
import ObjectDefinition from './admObjectDefinition.model.js'
import AccessLevelDefinition from './admAccessLevelDefinitions.model.js'

const AdmViewPermissions = sequelize.define(
	'adm_view_permission',
	{
		id: {
			type: Sequelize.INTEGER,
			autoIncrement: true,
			allowNull: false,
			primaryKey: true,
		},
		userId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'user_id',
		},
		roleId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'role_id',
		},
		functionalityDefinitionId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'adm_functionality_definition_id',
		},
		objectDefinitionId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'adm_object_definition_id',
		},
		accessLevelDefinitionId: {
			type: Sequelize.INTEGER,
			allowNull: false,
			field: 'adm_access_level_definition_id',
		},
	},
	{
		timestamps: false,
		tableName: 'adm_view_permission',
	}
)

AdmViewPermissions.belongsTo(FunctionalityDefinition, {
	as: 'functionalityDefinition',
	targetKey: 'id',
	foreignKey: 'functionalityDefinitionId',
})
AdmViewPermissions.belongsTo(ObjectDefinition, {
	as: 'objectDefinition',
	targetKey: 'id',
	foreignKey: 'objectDefinitionId',
})
AdmViewPermissions.belongsTo(AccessLevelDefinition, {
	as: 'accessLevelDefinition',
	targetKey: 'id',
	foreignKey: 'accessLevelDefinitionId',
})

export default AdmViewPermissions
