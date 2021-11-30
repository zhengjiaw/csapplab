truncate result.txt --size 0 
for i in {1..16}; do
    t=""
    if [ $i -ge 10 ]; then t="$i"
    else t="0$i"
    fi
    echo -e "./sdriver.pl  -t trace$t.txt -s ../tsh/tsh -a \"-p\"" >> result.txt
    ./sdriver.pl  -t trace$t.txt -s ../tsh/tsh -a "-p" >> result.txt
done