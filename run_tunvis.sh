#!/bin/bash

PID=0

trap ctrl_c INT
ctrl_c() {
    echo "** Trapped CTRL-C"
    kill $PID
    exit
}

while true; do
  ./tunvis &
  PID=$!
  inotifywait ./tunvis
  kill $PID
  sleep 2
done
