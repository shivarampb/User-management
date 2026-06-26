-- ---------------------------------------------------------------------------
--  User Management System - MariaDB schema
-- ---------------------------------------------------------------------------
CREATE DATABASE IF NOT EXISTS user_mgmt
  CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE user_mgmt;

CREATE TABLE IF NOT EXISTS userdetails (
    Username   VARCHAR(64)  NOT NULL,
    Password   VARCHAR(255) NOT NULL,
    Privilege  INT          NOT NULL,   -- 0 = Developer, 1 = Administrator, 2 = User
    PRIMARY KEY (Username)
);

-- Seed one Developer so you can log in the first time.
-- Credentials: dev / dev123
-- Password is stored as SHA3-256 (RealSha3_256) hex digest.
-- IMPORTANT: change this password immediately after first login.
INSERT INTO userdetails (Username, Password, Privilege)
VALUES ('dev', '53db5d7ef3f1a5d3492b07097933d1b0a5304a5c324aca9fdde9730c9787d24a', 0)
ON DUPLICATE KEY UPDATE Username = Username;

-- Optional sample accounts for testing:
-- INSERT INTO userdetails VALUES ('admin1', 'admin123', 1);
-- INSERT INTO userdetails VALUES ('user1',  'user123',  2);
