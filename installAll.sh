#! /bin/bash
mkdir csapplab
cd csapplab
labs=( "datalab" "bomblab" "attacklab" "archlab" "cachelab" "shlab" "malloclab" "proxylab" )
i=1
for dir in ${labs[*]} ;do
    lab="lab"$i"$dir"
    wget https://gitee.com/lin-xi-269/csapplab/raw/origin/$lab/install.sh -O install$lab.sh&& bash install$lab.sh && rm install$lab.sh
    i=$((i+1))
done

# lab2 cgdb 工具
# wget https://gitee.com/lin-xi-269/csapplab/raw/master/lab2bomb/installCgdb.sh
# bash installCgdb.sh

# lab4 环境配置
# wget https://gitee.com/lin-xi-269/csapplab/raw/master/lab4archlab/archlab-handout/installTclTk.sh && bash installTclTk.sh
