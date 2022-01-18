```bash
wget https://csapp.cs.cmu.edu/3e/ics3/code/conc/echoservert_pre.c -O echoservert_presOrigin.c
wget https://csapp.cs.cmu.edu/3e/ics3/code/conc/echo-cnt.c 
cp echoservert_preOrigin.c echoservert_pre.c  

是安全的，但不是可重入的，mutex使得其互斥安全，但是读写了mutex他就不是可重入的了。