#! /bin/bash
sudo apt-get install bison flex -y && sudo apt-get install tcl-dev tk-dev -y
installDIr="/tmp"
cd $installDIr
! ( test -e tcl8.5.19) &&  wget http://archive.ubuntu.com/ubuntu/pool/universe/t/tcl8.5/tcl8.5_8.5.19.orig.tar.gz && \
 tar -xvf tcl8.5_8.5.19.orig.tar.gz
! ( test -e tk8.5.19) && wget http://archive.ubuntu.com/ubuntu/pool/universe/t/tk8.5/tk8.5_8.5.19.orig.tar.gz && \
tar -xvf tk8.5_8.5.19.orig.tar.gz

cd /usr/include
( test -e tcl8.5) && sudo rm -rf tcl8.5
sudo mkdir tcl8.5
cd tcl8.5
sudo cp $installDIr/tk8.5.19/generic/* . -r
sudo cp /$installDIr/tcl8.5.19/generic/* . 

cd $installDIr
rm -rf tk8.5* tcl8.5*
