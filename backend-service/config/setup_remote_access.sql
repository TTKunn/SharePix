-- MySQL远程访问配置脚本
-- 允许root用户从任何IP连接

-- 创建或更新root用户的远程访问权限
CREATE USER IF NOT EXISTS 'root'@'%' IDENTIFIED BY 'Xzk200411.';

-- 授予所有权限
GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' WITH GRANT OPTION;

-- 修改认证插件为mysql_native_password（兼容旧版客户端）
ALTER USER 'root'@'%' IDENTIFIED WITH mysql_native_password BY 'Xzk200411.';

-- 刷新权限
FLUSH PRIVILEGES;

-- 查看授权结果
SELECT user, host, plugin FROM mysql.user WHERE user='root';

