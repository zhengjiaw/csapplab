truncate result --size 0 
for i in {1..16}; do
    t=""
    if [ $i -ge 10 ]; then t="$i"
    else t="0$i"
    fi
    echo "\n" >> result
    perl *.pl  -t trace$t.txt -s ../tsh/tsh -a 1 >> result.txt
done