#!/bin/bash

USERNAME=$USER
GTFSDB=gtfs_praha
OSMWALKDB=osmwalk-prepare

COMMAND="COPY (SELECT stop_id,stop_lat,stop_lon FROM gtfs_stops WHERE location_type=0) TO STDOUT"
psql -c "$COMMAND" $GTFSDB $USERNAME > stops.txt

COMMAND="TRUNCATE TABLE gtfs_stops"
psql -c "$COMMAND" $OSMWALKDB $USERNAME

COMMAND="COPY gtfs_stops FROM STDIN"
cat stops.txt | psql -c "$COMMAND" $OSMWALKDB $USERNAME
