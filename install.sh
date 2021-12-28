#! /bin/bash
labs=( "datalab" "bomblab" "attacklab" "archlab" "cachelab" "shlab" "malloclab" "proxylab" )
i=1
for dir in ${labs[*]} ;do
    lab="lab"$i"$dir"
    echo $lab
    ! ( test -d $lab ) && mkdir $lab
    cd $lab
    bash install.sh
    cd ..
    i=$((i+1))
done
