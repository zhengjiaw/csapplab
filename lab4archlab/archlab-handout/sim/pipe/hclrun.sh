make 
(cd ../y86-code; make testpsim)
(cd ../ptest; make SIM=../pipe/psim TFLAGS=-i)
