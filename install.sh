#! /bin/bash
labs=( "datalab" "bomblab" "attacklab" "archlab" "cachelab" "shlab" "malloclab" "proxylab" )
i=1
for dir in ${labs[*]} ;do
    lab="lab"$i"$dir"
    ! ( test -d $lab ) && mkdir $lab
    cd $lab
    wget https://gitee.com/lin-xi-269/csapplab/raw/origin/$lab/install.sh && bash install.sh && rm install.sh
    cd ..
    i=$((i+1))
done

