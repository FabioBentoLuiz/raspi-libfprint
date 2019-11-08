CREATE DATABASE IF NOT EXISTS libfprint;
USE libfprint;
CREATE TABLE IF NOT EXISTS `users`(`id` int(11) NOT NULL AUTO_INCREMENT,`name` varchar(45) DEFAULT NULL,`email` varchar(45) DEFAULT NULL,`fingerprint` blob,PRIMARY KEY (`id`));
GRANT ALL PRIVILEGES ON libfprint.* TO 'libfprint'@'%' IDENTIFIED BY '123456';
