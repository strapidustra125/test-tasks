#!/bin/bash

echo -e "\nRunning ZMQ zpplication...\n"

cd ..

./Release/zmq_app $1 $2 $3 $4 $5 $6 $7 $8 $9

cd scripts
