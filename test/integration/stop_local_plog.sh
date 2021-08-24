#!/bin/bash
#!/bin/bash
set -e
topname=$(dirname "$0")
cd ${topname}/../..

ulimit -n 1000000

if [ -s ./build/proc/nodepool ];
then
  # stop tso node
  sudo kill -9 $(cat ./build/proc/tso)
  cat /dev/null > ./build/proc/tso
  echo "Stop tso successfully!"
  # stop persistence node
  sudo kill -9 $(cat ./build/proc/persistence)
  cat /dev/null > ./build/proc/persistence
  echo "Stop persistence successfully!"
  # stop nodepool node
  sudo kill -9 $(cat ./build/proc/nodepool)
  cat /dev/null > ./build/proc/nodepool
  echo "Stop nodepool successfully!"
  # stop cpo node
  sudo kill -9 $(cat ./build/proc/cpo)
  cat /dev/null > ./build/proc/cpo
  echo "Stop cpo successfully!"

else
  echo "No node is running!"
fi

sudo rm -rf /mnt/pmem0/plog_data/*
echo "Delete all the plog"
sudo rm -rf /tmp/___cpo_integ_test
echo "Delete all the cpo"
