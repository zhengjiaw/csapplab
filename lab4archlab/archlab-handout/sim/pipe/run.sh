#! /bin/bash 
make VERSION=full  ## all
result=`./correctness.pl -p`
if [[ $result == *"68/68"* ]]; then
    result=`./correctness.pl`
    if [[ $result == *"68/68"* ]]; then
        ./benchmark.pl
    else 
        ./correctness.pl
    fi
else 
    ./correctness.pl -p
fi