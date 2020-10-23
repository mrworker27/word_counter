#!/bin/bash

IN_FILE=$1
OUT_FILE=$2

cp $IN_FILE "./in"

docker build . --tag=counter27 &> /dev/null
docker run -t counter27 > $OUT_FILE

rm "./in"
