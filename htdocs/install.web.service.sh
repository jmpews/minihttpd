wget http://download.redis.io/redis-stable.tar.gz
tar xzf redis-stable.tar.gz
cd redis-stable
make
./src/redis-server
# 安全配置
# rename-command CONFIG b840fc02d524045429941cc15f59e41cb7be6c52