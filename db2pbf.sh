#!/bin/sh
cd compiled
./todb.sh

cd ../postgis
./process.sh

cd ../compiled
./csvtograph ../data/

cd ../data
cp postgis-graph.pbf praha-graph.pbf
