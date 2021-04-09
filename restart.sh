#!/bin/bash

PID=0

trap ctrl_c INT
ctrl_c() {
    echo "** Trapped CTRL-C"
    kill $PID
    exit
}

for i in `seq 1 5`; do
  $@ &
  PID=$!
  inotifywait $1
  kill $PID
  sleep 2
done
