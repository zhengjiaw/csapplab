make drivers  ## ys
make psim VERSION=full ## hcl
make VERSION=full  ## all
./psim -t sdriver.yo  ##4
./psim -t ldriver.yo ## 63

../misc/yis sdriver.yo
 ./correctness.pl 

 (cd ../y86-code; make testpsim)
 (cd ../ptest; make SIM=../pipe/psim TFLAGS=-i) 
 ./correctness.pl -p 

 ./benchmark.pl


