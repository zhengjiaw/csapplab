#! /bin/bash
lab="malloclab"
if [ -d $lab ]; then labfile="$lab"_wget
else labfile="$lab"
fi
mkdir $labfile
cd  $labfile
wget http://csapp.cs.cmu.edu/3e/README-"$lab"
wget http://csapp.cs.cmu.edu/3e/"$lab".pdf
wget "https://gitee.com/lin-xi-269/csapplab/raw/origin/lab7$lab/$lab""_【彩云小译】.pdf"
wget http://csapp.cs.cmu.edu/3e/"$lab"-handout.tar
tar -xvf $lab-handout.tar
cd "$lab-handout"
wget "https://gitee.com/lin-xi-269/csapplab/raw/origin/lab7$lab/traces.tar.gz" && (tar -zxvf traces.tar.gz;rm traces.tar.gz)
