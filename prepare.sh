#!/bin/sh
#### Prepare data for searching ####
# Run ./ext-lib/mmpf/raptor/update-gtfs.sh prior to this script!

# Download OSM data, cut from them rectangle for Prague
# Download SRTM data and merge them to one file
cd osm
./prepare.sh

# Classify OSM data and load them to the database
cd ../compiled
./parse ../osm/praha.osm
./todb.sh

# Do most of the preparation work in PostgreSQL + PostGIS
cd ../postgis
./process.sh

# Make graph PBF file from the output of the database
cd ../compiled
./csvtograph ../data/

# Glacial relict from older preprocessing
cd ../data
cp postgis-graph.pbf praha-graph.pbf
