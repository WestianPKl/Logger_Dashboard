import { createBrowserRouter } from 'react-router'
import RootView, { loader as RootViewLoader, action as LogOutAction } from '../modules/Application/RootView'
import LoginPageView, { loader as LoginPageViewLoader } from '../modules/User/UserLoginPageView'
import RegisterPageView, { loader as RegisterPageViewLoader } from '../modules/User/UserRegisterPageView'
import UserPasswordResetLinkView, {
	loader as UserPasswordResetLinkViewLoader,
} from '../modules/User/UserPasswordResetLinkView'
import UserPasswordResetView, { loader as UserPasswordResetViewLoader } from '../modules/User/UserPasswordResetView'
import MainMenuView, { loader as MainMenuViewLoader } from '../modules/Application/MainMenuView'
import HouseMainView, { loader as HouseMainViewLoader } from '../modules/House/HouseMainView'
import HouseView, { loader as HouseViewLoader } from '../modules/House/HouseView'
import HouseFloorView, { loader as HouseFloorViewLoader } from '../modules/House/HouseFloorView'
import HouseLoggerView, { loader as HouseLoggerViewLoader } from '../modules/House/HouseLoggerView'
import HouseDetailsView, { loader as HouseDetailsViewLoader } from '../modules/HouseDetails/HouseDetailsView'
import EquipmentMainView, { loader as EquipmentMainViewLoader } from '../modules/Equipment/EquipmentMainView'
import UserMainView, { loader as UserMainViewLoader } from '../modules/User/UserMainView'
import UserProfileView, { loader as UserProfileViewLoader } from '../modules/User/UserProfileView'
import UserPermissionView, { loader as UserPermissionViewLoader } from '../modules/User/UserPermissionView'
import UserRolesView, { loader as UserRolesViewLoader } from '../modules/User/UserRolesView'
import DataLayoutView, { loader as DataLayoutViewLoader } from '../modules/Data/DataLayoutView'
import DataMainView, { loader as DataMainViewLoader } from '../modules/Data/DataMainView'
import DataChartView, { loader as DataChartViewLoader } from '../modules/Data/DataChartView'
import AdminMainView, { loader as AdminMainViewLoader } from '../modules/Admin/AdminMainView'
import AdminFunctionalityDefinitionView, {
	loader as AdminFunctionalityDefinitionViewLoader,
} from '../modules/Admin/AdminFunctionalityDefinitionView'
import AdminObjectDefinitionView, {
	loader as AdminObjectDefinitionViewLoader,
} from '../modules/Admin/AdminObjectDefinitionView'
import AdminAccessLevelDefinitionView, {
	loader as AdminAccessLevelDefinitionViewLoader,
} from '../modules/Admin/AdminAccessLevelDefinitionView'
import AdminPermissionRolesView, {
	loader as AdminPermissionRolesViewLoader,
} from '../modules/Admin/AdminPermissionRolesView'
import AdminUsersView, { loader as AdminUsersViewLoader } from '../modules/Admin/AdminUsersView'
import AdminEquipmentView, { loader as AdminEquipmentViewLoader } from '../modules/Admin/AdminEquipmentView'
import EquipmentVendorView, { loader as EquipmentVendorViewLoader } from '../modules/Equipment/EquipmentVendorView'
import EquipmentModelView, { loader as EquipmentModelViewLoader } from '../modules/Equipment/EquipmentModelView'
import EquipmentTypeView, { loader as EquipmentTypeViewLoader } from '../modules/Equipment/EquipmentTypeView'
import EquipmentLogView, { loader as EquipmentLogViewLoader } from '../modules/Equipment/EquipmentLogView'
import NotFoundView from '../modules/Application/NotFoundView'
import ErrorView from '../modules/Application/ErrorView'
import LoadingCircle from '../components/UI/LoadingCircle'
import DataDefinitionView, { loader as DataDefinitionViewLoader } from '../modules/Data/DataDefinitionView'

export const router = createBrowserRouter([
	{
		id: 'root',
		path: '/',
		loader: RootViewLoader,
		Component: RootView,
		ErrorBoundary: ErrorView,
		HydrateFallback: LoadingCircle,
		children: [
			{
				index: true,
				Component: MainMenuView,
				loader: MainMenuViewLoader,
			},
			{
				path: '/login',
				Component: LoginPageView,
				id: 'login',
				loader: LoginPageViewLoader,
			},
			{
				path: '/logout',
				action: LogOutAction,
			},
			{
				path: '/register',
				Component: RegisterPageView,
				id: 'register',
				loader: RegisterPageViewLoader,
			},
			{
				path: '/password-reset',
				Component: UserPasswordResetLinkView,
				id: 'password-reset-link',
				loader: UserPasswordResetLinkViewLoader,
			},
			{
				path: '/password-reset/:token',
				Component: UserPasswordResetView,
				id: 'password-reset',
				loader: UserPasswordResetViewLoader,
			},
			{
				path: '/house',
				Component: HouseMainView,
				id: 'house',
				loader: HouseMainViewLoader,
				children: [
					{ index: true, Component: HouseView, loader: HouseViewLoader },
					{ path: 'houses', Component: HouseView, loader: HouseViewLoader },
					{ path: 'floors', Component: HouseFloorView, loader: HouseFloorViewLoader },
					{ path: 'loggers', Component: HouseLoggerView, loader: HouseLoggerViewLoader },
				],
			},
			{
				path: 'house-details/:houseId',
				Component: HouseDetailsView,
				id: 'house-details',
				loader: HouseDetailsViewLoader,
			},
			{
				path: '/equipment',
				Component: EquipmentMainView,
				id: 'equipment',
				loader: EquipmentMainViewLoader,
			},
			{
				path: '/logs/:equLoggerId',
				Component: EquipmentLogView,
				id: 'equipment-logs',
				loader: EquipmentLogViewLoader,
			},
			{
				path: '/data',
				Component: DataLayoutView,
				id: 'data',
				loader: DataLayoutViewLoader,
				children: [
					{ index: true, Component: DataMainView, loader: DataMainViewLoader },
					{
						path: 'data-logger/:equLoggerId',
						Component: DataChartView,
						id: 'data-logger',
						loader: DataChartViewLoader,
					},
				],
			},
			{
				path: '/admin-panel',
				Component: AdminMainView,
				id: 'admin-panel',
				loader: AdminMainViewLoader,
				children: [
					{ index: true, Component: AdminFunctionalityDefinitionView, loader: AdminFunctionalityDefinitionViewLoader },
					{
						path: 'functionality-defnition',
						Component: AdminFunctionalityDefinitionView,
						loader: AdminFunctionalityDefinitionViewLoader,
					},
					{ path: 'object-definition', Component: AdminObjectDefinitionView, loader: AdminObjectDefinitionViewLoader },
					{
						path: 'access-levels-definition',
						Component: AdminAccessLevelDefinitionView,
						loader: AdminAccessLevelDefinitionViewLoader,
					},

					{
						path: 'permission-roles',
						Component: AdminPermissionRolesView,
						loader: AdminPermissionRolesViewLoader,
					},

					{
						path: 'users',
						Component: AdminUsersView,
						loader: AdminUsersViewLoader,
					},

					{
						path: 'equipment',
						Component: AdminEquipmentView,
						loader: AdminEquipmentViewLoader,
					},
					{
						path: 'equipment-vendors',
						Component: EquipmentVendorView,
						loader: EquipmentVendorViewLoader,
					},
					{
						path: 'equipment-models',
						Component: EquipmentModelView,
						loader: EquipmentModelViewLoader,
					},
					{
						path: 'equipment-types',
						Component: EquipmentTypeView,
						loader: EquipmentTypeViewLoader,
					},
					{
						path: 'data-definitions',
						Component: DataDefinitionView,
						loader: DataDefinitionViewLoader,
					},
				],
			},
			{
				path: '/profile',
				Component: UserMainView,
				loader: UserMainViewLoader,
				children: [
					{ index: true, Component: UserProfileView, loader: UserProfileViewLoader },
					{ path: 'user', Component: UserProfileView, loader: UserProfileViewLoader },
					{ path: 'permissions', Component: UserPermissionView, loader: UserPermissionViewLoader },
					{ path: 'roles', Component: UserRolesView, loader: UserRolesViewLoader },
				],
			},
			{
				path: '*',
				Component: NotFoundView,
				id: 'not-found',
			},
		],
	},
])
