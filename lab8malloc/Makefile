# 
# Makefile to manage the Malloc Lab.
#

# The name of the lab at your site (default: malloclab)
LABNAME = malloclab

# The handout directory where students will retrieve
# the $(LABNAME)-handout.tar file and the lab writeup.
#
DEST = <site specific>     

all: writeupfiles srcfiles handout handout-tarfile

writeupfiles:
	########################
	# Format the Lab writeup
	########################
	(cd writeup; make)

srcfiles:
	(cd ./src; make clean; make)

handout:
	###################################################
	# Copy the necessary files to the handout directory
	###################################################
	cp src/Makefile-handout $(LABNAME)-handout/Makefile
	(cd $(LABNAME)-handout; make clean)
	cp src/README-handout $(LABNAME)-handout/README
	cp src/clock.* $(LABNAME)-handout/
	cp src/config.h $(LABNAME)-handout/
	cp src/fcyc.* $(LABNAME)-handout/
	cp src/fsecs.* $(LABNAME)-handout/
	cp src/ftimer.* $(LABNAME)-handout/
	cp src/memlib.* $(LABNAME)-handout/
	cp src/mm-naive.c $(LABNAME)-handout/mm.c
	cp src/mm.h $(LABNAME)-handout/
	cp src/mdriver.c $(LABNAME)-handout/
	cp traces/short1-bal.rep $(LABNAME)-handout/
	cp traces/short2-bal.rep $(LABNAME)-handout/

handout-tarfile:
	########################################
	# Build tarfile of the handout directory
	########################################
	tar cvf $(LABNAME)-handout.tar $(LABNAME)-handout

dist:
	######################
	# Update distribution
	######################
	cp -p $(LABNAME)-handout.tar $(DEST)
	cp -p writeup/malloclab.ps $(DEST)/$(LABNAME).ps
	cp -p writeup/malloclab.pdf $(DEST)/$(LABNAME).pdf

clean:
	#####################################
	# Clean the entire lab directory tree
	#####################################
	rm -f *~ *.tar
	rm -rf $(LABNAME)-handout/*
	(cd src; make clean)
	(cd writeup; make clean)
