#!/bin/bash

USERNAME=$USER
RAPTOR_DIR=$(dirname $0)/../ext-lib/mmpf/raptor/
OSMWALKDB=osmwalk-prepare

COMMAND="TRUNCATE TABLE gtfs_stops"
psql -c "$COMMAND" $OSMWALKDB $USERNAME

COMMAND="COPY gtfs_stops(stop_id,raptor_id,stop_name,stop_lon,stop_lat,underground) FROM STDIN WITH CSV HEADER"
cat $RAPTOR_DIR/stopslut.csv | psql -c "$COMMAND" $OSMWALKDB $USERNAME
