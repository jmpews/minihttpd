# 更换源
# http://wiki.ubuntu.org.cn/Template:14.04source
sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak
# 设置编码
# sudo apt-get install language-pack-zh
# LANG="zh_CN.UTF-8" 
# LANGUAGE="zh_CN:zh" 
sudo locale-gen zh_CN.UTF-8

sudo apt-get install git

# install pyenv
sudo apt-get install libc6-dev gcc make build-essential libssl-dev zlib1g-dev libbz2-dev libreadline-dev libsqlite3-dev wget curl llvm
git clone git://github.com/yyuu/pyenv.git ~/.pyenv
echo 'export PYENV_ROOT="$HOME/.pyenv"' >> ~/.bashrc
echo 'export PATH="$PYENV_ROOT/bin:$PATH"' >> ~/.bashrc
echo 'eval "$(pyenv init -)"' >> ~/.bashrc
exec $SHELL -l
pyenv install 3.5.0 -v
