```bash
wget https://csapp.cs.cmu.edu/3e/ics3/code/conc/norace.c -O noraceOrigin.c --no-check-certificate
cp noraceOrigin.c norace.c  

是安全的，但不是可重入的，mutex使得其互斥安全，但是读写了mutex他就不是可重入的了。