-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Server version:               9.1.0 - MySQL Community Server - GPL
-- Server OS:                    Win64
-- HeidiSQL Version:             12.14.0.7165
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

-- Dumping structure for table logger_dev.adm_access_level_definition
CREATE TABLE IF NOT EXISTS `adm_access_level_definition` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(45) NOT NULL COMMENT 'Access level definition name.',
  `access_level` int NOT NULL COMMENT 'Definition access level.',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=20 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Access levels definition table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.adm_functionality_definition
CREATE TABLE IF NOT EXISTS `adm_functionality_definition` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(45) NOT NULL COMMENT 'Functionality definition name.',
  `description` varchar(255) NOT NULL COMMENT 'Functionality definition description.',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=17 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Functionality definition table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.adm_object_definition
CREATE TABLE IF NOT EXISTS `adm_object_definition` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(45) NOT NULL COMMENT 'Object definition name.',
  `description` varchar(255) NOT NULL COMMENT 'Object definition description.',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=27 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Object definition table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.adm_permissions
CREATE TABLE IF NOT EXISTS `adm_permissions` (
  `id` int NOT NULL AUTO_INCREMENT,
  `user_id` int DEFAULT NULL COMMENT 'User ID.',
  `role_id` int DEFAULT NULL COMMENT 'Adm Role ID.',
  `adm_functionality_definition_id` int NOT NULL COMMENT 'Functionality definition ID.',
  `adm_object_definition_id` int DEFAULT NULL COMMENT 'Object definition ID.',
  `adm_access_level_definition_id` int NOT NULL COMMENT 'Access level definition ID.',
  PRIMARY KEY (`id`),
  KEY `fk_permission_permission_user_idx` (`user_id`),
  KEY `fk_permission_permission_permission_role_idx` (`role_id`),
  KEY `fk_permission_permission_functionality_idx` (`adm_functionality_definition_id`),
  KEY `fk_permission_permission_object_definition_idx` (`adm_object_definition_id`),
  KEY `fk_permission_permission_permission_access_level_idx` (`adm_access_level_definition_id`),
  CONSTRAINT `fk_adm_permission_access_level` FOREIGN KEY (`adm_access_level_definition_id`) REFERENCES `adm_access_level_definition` (`id`),
  CONSTRAINT `fk_adm_permission_functionality` FOREIGN KEY (`adm_functionality_definition_id`) REFERENCES `adm_functionality_definition` (`id`),
  CONSTRAINT `fk_adm_permission_object_definition` FOREIGN KEY (`adm_object_definition_id`) REFERENCES `adm_object_definition` (`id`),
  CONSTRAINT `fk_adm_permission_role` FOREIGN KEY (`role_id`) REFERENCES `adm_roles` (`id`),
  CONSTRAINT `fk_adm_permisssion_user` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=56 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Permissions table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.adm_roles
CREATE TABLE IF NOT EXISTS `adm_roles` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(45) NOT NULL COMMENT 'Role template name.',
  `description` varchar(255) NOT NULL COMMENT 'Role template description.',
  `created_by_id` int NOT NULL,
  `updated_by_id` int NOT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `fk_adm_role_creator_users_idx` (`created_by_id`),
  KEY `fk_adm_role_editor_users_idx` (`updated_by_id`),
  CONSTRAINT `fk_adm_roles_creator_users` FOREIGN KEY (`created_by_id`) REFERENCES `users` (`id`),
  CONSTRAINT `fk_adm_roles_editor_users` FOREIGN KEY (`updated_by_id`) REFERENCES `users` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Role templates table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.adm_roles_user
CREATE TABLE IF NOT EXISTS `adm_roles_user` (
  `role_id` int NOT NULL COMMENT 'Adm Role ID.',
  `user_id` int NOT NULL COMMENT 'User ID.',
  PRIMARY KEY (`role_id`,`user_id`),
  KEY `fk_adm_roles_users_idx` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Role templates users table.';

-- Data exporting was unselected.

-- Dumping structure for view logger_dev.adm_view_permission
-- Creating temporary table to overcome VIEW dependency errors
CREATE TABLE `adm_view_permission` (
	`id` INT NOT NULL,
	`user_id` INT NULL COMMENT 'User ID.',
	`role_id` INT NULL COMMENT 'Adm Role ID.',
	`adm_functionality_definition_id` INT NOT NULL COMMENT 'Functionality definition ID.',
	`adm_object_definition_id` INT NULL COMMENT 'Object definition ID.',
	`adm_access_level_definition_id` INT NOT NULL COMMENT 'Access level definition ID.'
);

-- Dumping structure for table logger_dev.data_definitions
CREATE TABLE IF NOT EXISTS `data_definitions` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(45) NOT NULL COMMENT 'Data definition name.',
  `unit` varchar(20) NOT NULL COMMENT 'Data definition unit.',
  `description` varchar(200) NOT NULL COMMENT 'Data definition description.',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Data definitions table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.data_last_value
CREATE TABLE IF NOT EXISTS `data_last_value` (
  `id` int NOT NULL AUTO_INCREMENT,
  `data_log_id` int NOT NULL COMMENT 'Data Log ID.',
  `equ_logger_id` int NOT NULL,
  `equ_sensor_id` int NOT NULL,
  `data_definition_id` int NOT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `fk_data_last_value_data_logs_idx` (`data_log_id`),
  KEY `fk_data_last_value_equ_logger_idx` (`equ_logger_id`),
  KEY `fk_data_last_value_equ_sensor_idx` (`equ_sensor_id`),
  KEY `fk_data_last_value_data_definition_idx` (`data_definition_id`),
  CONSTRAINT `fk_data_last_value_data_definition` FOREIGN KEY (`data_definition_id`) REFERENCES `data_definitions` (`id`),
  CONSTRAINT `fk_data_last_value_data_logs` FOREIGN KEY (`data_log_id`) REFERENCES `data_logs` (`id`),
  CONSTRAINT `fk_data_last_value_equ_logger` FOREIGN KEY (`equ_logger_id`) REFERENCES `equ_equipment` (`id`),
  CONSTRAINT `fk_data_last_value_equ_sensor` FOREIGN KEY (`equ_sensor_id`) REFERENCES `equ_equipment` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=247150 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Data last value table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.data_logs
CREATE TABLE IF NOT EXISTS `data_logs` (
  `id` int NOT NULL AUTO_INCREMENT,
  `value` varchar(45) NOT NULL COMMENT 'Data value.',
  `data_definition_id` int NOT NULL COMMENT 'Data type ID.',
  `equ_logger_id` int NOT NULL COMMENT 'Equipment logger ID.',
  `equ_sensor_id` int NOT NULL,
  `time` varchar(255) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_data_logs_equ_equipment_idx` (`equ_logger_id`),
  KEY `fk_data_logs_data_type_idx` (`data_definition_id`),
  KEY `fk_data_logs_equ_equipment_sensor_idx` (`equ_sensor_id`),
  CONSTRAINT `fk_data_logs_data_type` FOREIGN KEY (`data_definition_id`) REFERENCES `data_definitions` (`id`),
  CONSTRAINT `fk_data_logs_equ_equipment` FOREIGN KEY (`equ_logger_id`) REFERENCES `equ_equipment` (`id`),
  CONSTRAINT `fk_data_logs_equ_equipment_sensor` FOREIGN KEY (`equ_sensor_id`) REFERENCES `equ_equipment` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=247158 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Data logs table.';

-- Data exporting was unselected.

-- Dumping structure for view logger_dev.data_view_connected_sensor
-- Creating temporary table to overcome VIEW dependency errors
CREATE TABLE `data_view_connected_sensor` (
	`equ_logger_id` INT NULL COMMENT 'Equipment logger ID.',
	`equ_sensor_id` INT NULL,
	`house_floor_id` INT NULL,
	`house_logger_id` INT NULL,
	`sensor_vendor` VARCHAR(1) NULL COMMENT 'Equipment vendor name.' COLLATE 'utf8mb4_0900_ai_ci',
	`sensor_model` VARCHAR(1) NULL COMMENT 'Equipment model name.' COLLATE 'utf8mb4_0900_ai_ci',
	`sensor_serial_number` VARCHAR(1) NULL COMMENT 'Equipment serial number.' COLLATE 'utf8mb4_0900_ai_ci'
);

-- Dumping structure for view logger_dev.data_view_last_value
-- Creating temporary table to overcome VIEW dependency errors
CREATE TABLE `data_view_last_value` (
	`id` INT NOT NULL,
	`equ_logger_id` INT NULL COMMENT 'Equipment logger ID.',
	`equ_sensor_id` INT NULL,
	`house_floor_id` INT NULL,
	`house_logger_id` INT NULL,
	`time` VARCHAR(1) NULL COLLATE 'utf8mb4_0900_ai_ci',
	`value` VARCHAR(1) NULL COMMENT 'Data value.' COLLATE 'utf8mb4_0900_ai_ci',
	`parameter` VARCHAR(1) NULL COMMENT 'Data definition name.' COLLATE 'utf8mb4_0900_ai_ci',
	`unit` VARCHAR(1) NULL COMMENT 'Data definition unit.' COLLATE 'utf8mb4_0900_ai_ci'
);

-- Dumping structure for view logger_dev.data_view_logs
-- Creating temporary table to overcome VIEW dependency errors
CREATE TABLE `data_view_logs` (
	`time` VARCHAR(1) NOT NULL COLLATE 'utf8mb4_0900_ai_ci',
	`equ_logger_id` INT NOT NULL COMMENT 'Equipment logger ID.',
	`equ_sensor_id` INT NOT NULL,
	`temperature` VARCHAR(1) NULL COLLATE 'utf8mb4_0900_ai_ci',
	`humidity` VARCHAR(1) NULL COLLATE 'utf8mb4_0900_ai_ci',
	`atmPressure` VARCHAR(1) NULL COLLATE 'utf8mb4_0900_ai_ci',
	`altitude` VARCHAR(1) NULL COLLATE 'utf8mb4_0900_ai_ci'
);

-- Dumping structure for table logger_dev.equ_equipment
CREATE TABLE IF NOT EXISTS `equ_equipment` (
  `id` int NOT NULL AUTO_INCREMENT,
  `serial_number` varchar(255) NOT NULL COMMENT 'Equipment serial number.',
  `equ_vendor_id` int NOT NULL COMMENT 'Equipment vendor ID.',
  `equ_model_id` int NOT NULL COMMENT 'Equipment model ID.',
  `equ_type_id` int NOT NULL COMMENT 'Equipment type ID.',
  `created_by_id` int NOT NULL COMMENT 'Creator ID.',
  `updated_by_id` int NOT NULL COMMENT 'Editor ID.',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `deleted_at` datetime DEFAULT NULL,
  `is_deleted` tinyint DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `fk_equ_equipment_equ_type_idx` (`equ_type_id`),
  KEY `fk_equ_equipment_created_by_idx` (`created_by_id`),
  KEY `fk_equ_equipment_updated_by_idx` (`updated_by_id`),
  KEY `fk_equ_equipment_equ_vendor_idx` (`equ_vendor_id`),
  KEY `fk_equ_equipment_equ_model_idx` (`equ_model_id`),
  CONSTRAINT `fk_equ_equipment_created_by` FOREIGN KEY (`created_by_id`) REFERENCES `users` (`id`),
  CONSTRAINT `fk_equ_equipment_equ_model` FOREIGN KEY (`equ_model_id`) REFERENCES `equ_model` (`id`),
  CONSTRAINT `fk_equ_equipment_equ_type` FOREIGN KEY (`equ_type_id`) REFERENCES `equ_type` (`id`),
  CONSTRAINT `fk_equ_equipment_equ_vendor` FOREIGN KEY (`equ_vendor_id`) REFERENCES `equ_vendor` (`id`),
  CONSTRAINT `fk_equ_equipment_updated_by` FOREIGN KEY (`updated_by_id`) REFERENCES `users` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=380 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Equipment list table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.equ_log
CREATE TABLE IF NOT EXISTS `equ_log` (
  `id` int NOT NULL AUTO_INCREMENT,
  `equipment_id` int NOT NULL,
  `message` varchar(255) DEFAULT NULL,
  `type` enum('STATUS','DATA','OTA','ERROR','OTHER') DEFAULT NULL,
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `fk_equ_log_equ_equipment_idx` (`equipment_id`),
  CONSTRAINT `fk_equ_log_equ_equipment` FOREIGN KEY (`equipment_id`) REFERENCES `equ_equipment` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=32 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.equ_model
CREATE TABLE IF NOT EXISTS `equ_model` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL COMMENT 'Equipment model name.',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=21 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Equipment model table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.equ_sensor_functions
CREATE TABLE IF NOT EXISTS `equ_sensor_functions` (
  `equ_sensor_id` int NOT NULL,
  `data_definition_id` int NOT NULL,
  PRIMARY KEY (`equ_sensor_id`,`data_definition_id`),
  KEY `fk_equ_sensor_functions_data_definiton_idx` (`data_definition_id`),
  CONSTRAINT `fk_equ_sensor_functions_data_definiton` FOREIGN KEY (`data_definition_id`) REFERENCES `data_definitions` (`id`),
  CONSTRAINT `fk_equ_sensor_functions_equ_equipment` FOREIGN KEY (`equ_sensor_id`) REFERENCES `equ_equipment` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.equ_stats
CREATE TABLE IF NOT EXISTS `equ_stats` (
  `id` int NOT NULL AUTO_INCREMENT,
  `equipment_id` int NOT NULL,
  `last_seen` datetime NOT NULL,
  `sn_contr` varchar(100) DEFAULT NULL,
  `fw_contr` varchar(45) DEFAULT NULL,
  `hw_contr` varchar(45) DEFAULT NULL,
  `build_contr` varchar(45) DEFAULT NULL,
  `prod_contr` varchar(45) DEFAULT NULL,
  `sn_com` varchar(100) DEFAULT NULL,
  `fw_com` varchar(45) DEFAULT NULL,
  `hw_com` varchar(45) DEFAULT NULL,
  `build_com` varchar(45) DEFAULT NULL,
  `prod_com` varchar(45) DEFAULT NULL,
  `ip_address` varchar(150) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_equ_stats_equ_equipment_idx` (`equipment_id`),
  CONSTRAINT `fk_equ_stats_equ_equipment` FOREIGN KEY (`equipment_id`) REFERENCES `equ_equipment` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3045 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.equ_type
CREATE TABLE IF NOT EXISTS `equ_type` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL COMMENT 'Equipment type name.',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Equipment type table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.equ_vendor
CREATE TABLE IF NOT EXISTS `equ_vendor` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL COMMENT 'Equipment vendor name.',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=18 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Equipment vendor table.';

-- Data exporting was unselected.

-- Dumping structure for view logger_dev.equ_view_unused_logger
-- Creating temporary table to overcome VIEW dependency errors
CREATE TABLE `equ_view_unused_logger` (
	`equ_logger_id` INT NOT NULL
);

-- Dumping structure for table logger_dev.error_log
CREATE TABLE IF NOT EXISTS `error_log` (
  `id` int NOT NULL AUTO_INCREMENT,
  `message` varchar(255) NOT NULL,
  `details` varchar(255) DEFAULT NULL,
  `type` enum('Equipment','Other','DB') NOT NULL,
  `severity` enum('Critical','Error','Warning','Info') NOT NULL,
  `equipment_id` int DEFAULT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `fk_error_log_equ_equipment_idx` (`equipment_id`),
  CONSTRAINT `fk_error_log_equ_equipment` FOREIGN KEY (`equipment_id`) REFERENCES `equ_equipment` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=937 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.house_floors
CREATE TABLE IF NOT EXISTS `house_floors` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(45) NOT NULL COMMENT 'House floor name.',
  `layout` varchar(255) DEFAULT NULL,
  `layout_big` varchar(255) DEFAULT NULL,
  `house_id` int NOT NULL COMMENT 'House id.',
  `x` float DEFAULT '0',
  `y` float DEFAULT '0',
  `zoom` float DEFAULT '1',
  `pos_x` float DEFAULT '0',
  `pos_y` float DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `fk_house_floors_house_house_idx` (`house_id`),
  CONSTRAINT `fk_house_floors_house_house` FOREIGN KEY (`house_id`) REFERENCES `house_house` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=17 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='House floors table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.house_house
CREATE TABLE IF NOT EXISTS `house_house` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL COMMENT 'House name.',
  `postal_code` varchar(45) DEFAULT NULL COMMENT 'House localization postal code.',
  `city` varchar(45) DEFAULT NULL,
  `street` varchar(255) DEFAULT NULL COMMENT 'House localization street.',
  `house_number` varchar(45) DEFAULT NULL COMMENT 'House localization street/house number.',
  `picture_link` varchar(255) DEFAULT NULL,
  `picture_link_big` varchar(255) DEFAULT NULL,
  `created_by_id` int NOT NULL,
  `updated_by_id` int NOT NULL,
  `created_at` datetime DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `fk_house_house_creator_users_idx` (`created_by_id`),
  KEY `fk_house_house_editor_users_idx` (`updated_by_id`),
  CONSTRAINT `fk_house_house_creator_users` FOREIGN KEY (`created_by_id`) REFERENCES `users` (`id`),
  CONSTRAINT `fk_house_house_editor_users` FOREIGN KEY (`updated_by_id`) REFERENCES `users` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=60 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='House table';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.house_logger
CREATE TABLE IF NOT EXISTS `house_logger` (
  `id` int NOT NULL AUTO_INCREMENT,
  `equ_logger_id` int NOT NULL COMMENT 'Equipment logger ID.',
  `house_floor_id` int NOT NULL,
  `pos_x` float DEFAULT '0',
  `pos_y` float DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `equ_logger_id_UNIQUE` (`equ_logger_id`),
  KEY `fk_house_logger_equ_equipment_idx` (`equ_logger_id`),
  KEY `fk_house_logger_house_floors_idx` (`house_floor_id`),
  CONSTRAINT `fk_house_logger_equ_equipment` FOREIGN KEY (`equ_logger_id`) REFERENCES `equ_equipment` (`id`),
  CONSTRAINT `fk_house_logger_house_floors` FOREIGN KEY (`house_floor_id`) REFERENCES `house_floors` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=38 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='House loggers table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.process_definition
CREATE TABLE IF NOT EXISTS `process_definition` (
  `id` int NOT NULL AUTO_INCREMENT,
  `process_type_id` int NOT NULL,
  `name` varchar(255) NOT NULL,
  `created_by_id` int NOT NULL,
  `updated_by_id` int NOT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `fk_process_definition_creator_users_idx` (`created_by_id`),
  KEY `fk_process_definition_editor_users_idx` (`updated_by_id`),
  CONSTRAINT `fk_process_definition_creator_users` FOREIGN KEY (`created_by_id`) REFERENCES `users` (`id`),
  CONSTRAINT `fk_process_definition_editor_users` FOREIGN KEY (`updated_by_id`) REFERENCES `users` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Process definition table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.process_type
CREATE TABLE IF NOT EXISTS `process_type` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL COMMENT 'Process type name.',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Process type table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.superusers
CREATE TABLE IF NOT EXISTS `superusers` (
  `id` int NOT NULL AUTO_INCREMENT,
  `user_id` int DEFAULT NULL COMMENT 'User ID.',
  PRIMARY KEY (`id`),
  KEY `fk_superusers_users_idx` (`user_id`),
  CONSTRAINT `fk_superusers_users` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Superusers table.';

-- Data exporting was unselected.

-- Dumping structure for table logger_dev.users
CREATE TABLE IF NOT EXISTS `users` (
  `id` int NOT NULL AUTO_INCREMENT,
  `username` varchar(45) NOT NULL,
  `email` varchar(255) NOT NULL,
  `password` varchar(255) NOT NULL,
  `confirmed` tinyint DEFAULT '0',
  `avatar` varchar(255) DEFAULT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `avatar_big` varchar(255) DEFAULT NULL,
  `reset_password_token` varchar(255) DEFAULT NULL,
  `reset_password_expires` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `email_UNIQUE` (`email`),
  UNIQUE KEY `username_UNIQUE` (`username`)
) ENGINE=InnoDB AUTO_INCREMENT=23 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='Users list table.';

-- Data exporting was unselected.

-- Removing temporary table and create final VIEW structure
DROP TABLE IF EXISTS `adm_view_permission`;
CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `adm_view_permission` AS select `p`.`id` AS `id`,`p`.`user_id` AS `user_id`,`p`.`role_id` AS `role_id`,`p`.`adm_functionality_definition_id` AS `adm_functionality_definition_id`,`p`.`adm_object_definition_id` AS `adm_object_definition_id`,`p`.`adm_access_level_definition_id` AS `adm_access_level_definition_id` from (`adm_permissions` `p` left join `adm_roles` `r` on((`r`.`id` = `p`.`role_id`)))
;

-- Removing temporary table and create final VIEW structure
DROP TABLE IF EXISTS `data_view_connected_sensor`;
CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `data_view_connected_sensor` AS select `v`.`equ_logger_id` AS `equ_logger_id`,`v`.`equ_sensor_id` AS `equ_sensor_id`,`h`.`house_floor_id` AS `house_floor_id`,`h`.`id` AS `house_logger_id`,`sv`.`name` AS `sensor_vendor`,`sm`.`name` AS `sensor_model`,`s`.`serial_number` AS `sensor_serial_number` from (((((`data_last_value` `vl` left join `data_logs` `v` on((`v`.`id` = `vl`.`data_log_id`))) left join `house_logger` `h` on((`h`.`equ_logger_id` = `v`.`equ_logger_id`))) left join `equ_equipment` `s` on((`s`.`id` = `v`.`equ_sensor_id`))) left join `equ_vendor` `sv` on((`sv`.`id` = `s`.`equ_vendor_id`))) left join `equ_model` `sm` on((`sm`.`id` = `s`.`equ_model_id`))) group by `v`.`equ_logger_id`,`v`.`equ_sensor_id`,`sv`.`name`,`sm`.`name`,`s`.`serial_number`,`h`.`house_floor_id`,`h`.`id` order by `v`.`equ_sensor_id`
;

-- Removing temporary table and create final VIEW structure
DROP TABLE IF EXISTS `data_view_last_value`;
CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `data_view_last_value` AS select `vl`.`id` AS `id`,`v`.`equ_logger_id` AS `equ_logger_id`,`v`.`equ_sensor_id` AS `equ_sensor_id`,`h`.`house_floor_id` AS `house_floor_id`,`h`.`id` AS `house_logger_id`,`v`.`time` AS `time`,`v`.`value` AS `value`,`d`.`name` AS `parameter`,`d`.`unit` AS `unit` from (((`data_last_value` `vl` left join `data_logs` `v` on((`v`.`id` = `vl`.`data_log_id`))) left join `house_logger` `h` on((`h`.`equ_logger_id` = `v`.`equ_logger_id`))) left join `data_definitions` `d` on((`d`.`id` = `v`.`data_definition_id`))) order by `v`.`equ_sensor_id`
;

-- Removing temporary table and create final VIEW structure
DROP TABLE IF EXISTS `data_view_logs`;
CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `data_view_logs` AS select `v`.`time` AS `time`,`v`.`equ_logger_id` AS `equ_logger_id`,`v`.`equ_sensor_id` AS `equ_sensor_id`,max((case when (`d`.`name` = 'temperature') then replace(`v`.`value`,',','.') end)) AS `temperature`,max((case when (`d`.`name` = 'humidity') then replace(`v`.`value`,',','.') end)) AS `humidity`,max((case when (`d`.`name` = 'atmPressure') then replace(`v`.`value`,',','.') end)) AS `atmPressure`,max((case when (`d`.`name` = 'altitude') then replace(`v`.`value`,',','.') end)) AS `altitude` from (`data_logs` `v` left join `data_definitions` `d` on((`d`.`id` = `v`.`data_definition_id`))) group by `v`.`time`,`v`.`equ_logger_id`,`v`.`equ_sensor_id` order by `v`.`time`
;

-- Removing temporary table and create final VIEW structure
DROP TABLE IF EXISTS `equ_view_unused_logger`;
CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `equ_view_unused_logger` AS select `e`.`id` AS `equ_logger_id` from (`equ_equipment` `e` join `equ_type` `te` on((`e`.`equ_type_id` = `te`.`id`))) where ((`te`.`name` = 'logger') and (`e`.`is_deleted` = 0) and `e`.`id` in (select `house_logger`.`equ_logger_id` from `house_logger` where (`house_logger`.`equ_logger_id` is not null)) is false)
;

/*!40103 SET TIME_ZONE=IFNULL(@OLD_TIME_ZONE, 'system') */;
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IFNULL(@OLD_FOREIGN_KEY_CHECKS, 1) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40111 SET SQL_NOTES=IFNULL(@OLD_SQL_NOTES, 1) */;
