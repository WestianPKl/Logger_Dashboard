import { body } from 'express-validator'
import User from '../api/model/users/user.model.js'

export const usernameLogin = body('username', 'Please insert correct data!')
    .trim()
    .notEmpty()

export const emailLogin = body('email', 'Please insert correct data!')
    .trim()
    .notEmpty()
    .isEmail()
    .withMessage('Invalid email address')

export const passwordLogin = body('password', 'Please insert correct data!')
    .notEmpty()
    .isLength({ min: 3 })
    .withMessage('Password is too short (at least 3 characters)!')

export const usernameRegister = body('username')
    .trim()
    .notEmpty()
    .withMessage('Please insert correct data!')
    .custom(async (value) => {
        const userDoc = await User.findOne({ where: { username: value } })
        if (userDoc) {
            return Promise.reject('Username exists already!')
        }
    })

export const emailRegister = body('email')
    .notEmpty()
    .trim()
    .isEmail()
    .withMessage('Invalid email address!')
    .custom(async (value) => {
        const userDoc = await User.findOne({ where: { email: value } })
        if (userDoc) {
            return Promise.reject('Email exists already!')
        }
    })

export const passwordRegister = body('password')
    .notEmpty()
    .withMessage('Please insert correct data!')
    .isLength({ min: 3 })
    .withMessage('Password is too short (at least 3 characters)!')

export const confirmPasswordRegister = body(
    'confirmPassword',
    'Please insert correct data!'
)
    .notEmpty()
    .custom((value, { req }) => {
        if (value !== req.body.password) {
            throw new Error('Passwords have to match!')
        }
        return true
    })

export const dataName = body('name', 'Please insert correct data!').notEmpty()
export const dataUnit = body('unit', 'Please insert correct data!').notEmpty()
export const dataValue = body('value', 'Please insert correct data!').notEmpty()
export const dataDescription = body(
    'description',
    'Please insert correct data!'
).notEmpty()
export const dataDataDefinitionId = body(
    'dataDefinitionId',
    'Please insert correct data!'
).notEmpty()
export const dataDataLogId = body(
    'dataLogId',
    'Please insert correct data!'
).notEmpty()

export const processProcessTypeId = body(
    'processTypeId',
    'Please insert correct data!'
).notEmpty()

export const admAccessLevel = body(
    'accessLevel',
    'Please insert correct data!'
).notEmpty()
export const houseFloorId = body(
    'floorId',
    'Please insert correct data!'
).notEmpty()
export const housePostalCode = body(
    'postalCode',
    'Please insert correct data!'
).notEmpty()
export const houseStreet = body(
    'street',
    'Please insert correct data!'
).notEmpty()
export const houseNumber = body(
    'houseNumber',
    'Please insert correct data!'
).notEmpty()
export const houseLayout = body(
    'layout',
    'Please insert correct data!'
).notEmpty()
export const houseHouseId = body(
    'houseId',
    'Please insert correct data!'
).notEmpty()
export const admRoleId = body(
    'roleId',
    'Please insert correct data!'
).notEmpty()
export const admUserId = body(
    'userId',
    'Please insert correct data!'
).notEmpty()
export const admFunctionalityDefinitionId = body(
    'admFunctionalityDefinitionId',
    'Please insert correct data!'
).notEmpty()
export const admObjectDefinitionId = body(
    'admObjectDefinitionId',
    'Please insert correct data!'
).notEmpty()
export const admAccessLevelDefinitionId = body(
    'admAccessLevelDefinitionId',
    'Please insert correct data!'
).notEmpty()

export const equipmentSerialNumber = body('serialNumber').notEmpty()
export const equipmentVendor = body('equVendorId').notEmpty().isNumeric()
export const equipmentModel = body('equModelId').notEmpty().isNumeric()
export const equipmentType = body('equTypeId').notEmpty().isNumeric()

export const houseEquLogger = body('equLoggerId').notEmpty()
export const equSensor = body('equSensorId').notEmpty()
export const houseLoggerId = body('houseLoggerId').notEmpty()

export const usernameReset = body('username')
    .trim()
    .notEmpty()
    .withMessage('Please insert correct data!')

export const emailReset = body('email')
    .notEmpty()
    .trim()
    .isEmail()
    .withMessage('Invalid email address!')

export const errorMessage = body(
    'message',
    'Please insert correct data!'
).notEmpty()
export const errorType = body('type', 'Please insert correct data!').notEmpty()
export const errorSeverity = body(
    'severity',
    'Please insert correct data!'
).notEmpty()
