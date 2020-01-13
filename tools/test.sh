#!/bin/sh

set -e

[ -d /etc/supla-server ] || sudo mkdir /etc/supla-server
[ -e /etc/supla-server/supla-test.cfg ] || sudo touch /etc/supla-server/supla-test.cfg

vg_verify() {
   if ! grep "All heap blocks were freed -- no leaks are possible" ./vg-test.log; then
     cat ./vg-test.log
     rm ./vg-test.log
     echo "Memory leak error!"
     exit 1
   fi
   
   rm -f ./vg-test.log
}

cd supla-server/Test 
make clean && make all 
./supla-server 

valgrind --version
../../tools/valgrind-full.sh ./supla-server > /dev/null 2> vg-test.log
vg_verify

cd ../Release 
make clean && make all 
cd ../../supla-scheduler/Test 
make clean && make all 
./supla-scheduler

../../tools/valgrind-full.sh ./supla-scheduler > /dev/null 2> vg-test.log
vg_verify

cd ../Release 
make clean && make all 
cd ../../supla-dev/Release 
make clean && make 
cd ../../supla-console-client/Debug 
make clean && make 

echo OK 
exit 0
