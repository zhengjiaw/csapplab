#! /bin/bash
# You must have the following packages installed.
    # sh
    # autoconf
    # automake
    # aclocal
    # autoheader
    # libtool
    # flex
    # bison
    # gcc/g++ (c11/c++11 support)
cd /tmp
! ( test -d cgdb) && git clone https://github.com.cnpmjs.org/cgdb/cgdb.git  #这是加速后的镜像
# 下面是源镜像 https://github.com/cgdb/cgdb.git

cd cgdb
build="qweasd13sdfx12esdui1"
sudo ./autogen.sh 
( test -d ../$build) && sudo rm -rf ../$build
mkdir ../$build
cd ../$build
sudo apt-get update
sudo apt-get install flex -y
sudo apt-get install texinfo -y
sudo apt-get install libreadline-dev -y
sudo ../cgdb/configure --prefix=$PWD/../prefix
sudo make -srj4
sudo make install

cd cgdb
sudo mv ./cgdb  /usr/local/bin/

# 这里大概设置了搜索大小写不敏感，tab 大小，分屏方式（默认改为左右分屏）：
echo -e "set ignorecase\nset ts=4\nset wso=vertical\nset eld=shortarrow\nset hls\n" > ~/.cgdb/cgdbrc
cd /tmp
sudo rm -rf $build/ cgdb/ prefix/