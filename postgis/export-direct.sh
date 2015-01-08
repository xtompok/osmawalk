#!/bin/sh

COMMAND="COPY direct(id1,id2) TO STDOUT DELIMITER ';' CSV HEADER;"
psql -c "$COMMAND" osmwalk-prepare jethro >direct.csv  
