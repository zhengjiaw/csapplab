#! /bin/bash

for file in *;do
    ! ( test -d $file ) && continue
    cd $file
    var="$file"
    if   test ${var:0:1} = "5";then
        ../hex2raw < hex | ../rtarget  -q
    else 
        ../hex2raw < hex | ../ctarget  -q
    fi
    cd ..
    echo ""
done

