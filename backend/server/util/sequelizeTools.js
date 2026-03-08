import Sequelize from 'sequelize'
const Op = Sequelize.Op

const opMap = {
	$or: Op.or,
	$and: Op.and,
	$like: Op.like,
	$between: Op.between,
	$in: Op.in,
	$eq: Op.eq,
	$ne: Op.ne,
	$is: Op.is,
	$not: Op.not,
	$gte: Op.gte,
	$gt: Op.gt,
	$lt: Op.lt,
	$lte: Op.lte,
	$col: Op.col,
	$notIn: Op.notIn,
	$notLike: Op.notLike,
	$notBetween: Op.notBetween,
	$overlap: Op.overlap,
	$contains: Op.contains,
	$contained: Op.contained,
	$any: Op.any,
	$all: Op.all,
	$values: Op.values,
	$fn: Op.fn,
	$literal: Op.literal,
	$cast: Op.cast,
	$regexp: Op.regexp,
	$notRegexp: Op.notRegexp,
	$iRegexp: Op.iRegexp,
	$notIRegexp: Op.notIRegexp,
}

export function encodeSequelizeQuery(queryObject = {}) {
	if (Array.isArray(queryObject)) {
		return queryObject.map((item) => encodeSequelizeQuery(item))
	}
	if (typeof queryObject !== 'object' || queryObject === null) {
		return queryObject
	}

	let result = {}
	for (const key in queryObject) {
		const value = queryObject[key]
		if (Object.values(Op).includes(key)) {
			const opKey = Object.keys(Op).find((k) => Op[k] === key)
			result[`$${opKey}`] = encodeSequelizeQuery(value)
		} else if (
			typeof value === 'object' &&
			value !== null &&
			!Array.isArray(value)
		) {
			result[key] = {}
			for (const subKey in value) {
				if (Object.values(Op).includes(subKey)) {
					const opSubKey = Object.keys(Op).find(
						(k) => Op[k] === subKey
					)
					result[key][`$${opSubKey}`] = encodeSequelizeQuery(
						value[subKey]
					)
				} else {
					result[key][subKey] = encodeSequelizeQuery(value[subKey])
				}
			}
		} else {
			result[key] = encodeSequelizeQuery(value)
		}
	}
	return result
}

export function decodeSequelizeQuery(queryObject = {}) {
	if (Array.isArray(queryObject)) {
		return queryObject.map((item) => decodeSequelizeQuery(item))
	}
	if (typeof queryObject !== 'object' || queryObject === null) {
		return queryObject
	}

	let result = {}
	for (const key in queryObject) {
		const value = queryObject[key]

		if (opMap[key]) {
			result[opMap[key]] = decodeSequelizeQuery(value)
		} else if (
			typeof value === 'object' &&
			value !== null &&
			!Array.isArray(value)
		) {
			result[key] = {}
			for (const subKey in value) {
				if (opMap[subKey]) {
					result[key][opMap[subKey]] = decodeSequelizeQuery(
						value[subKey]
					)
				} else {
					result[key][subKey] = decodeSequelizeQuery(value[subKey])
				}
			}
		} else {
			result[key] = decodeSequelizeQuery(value)
		}
	}
	return result
}
