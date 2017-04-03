#!/bin/bash

USERNAME=$USER
RAPTOR_DIR=../
OSMWALKDB=osmwalk-prepare

COMMAND="TRUNCATE TABLE gtfs_stops"
psql -c "$COMMAND" $OSMWALKDB $USERNAME

COMMAND="COPY gtfs_stops FROM STDIN"
cat stops.txt | psql -c "$COMMAND" $OSMWALKDB $USERNAME
