#!/bin/bash

USERNAME=$USER
DATABASE=osmwalk-prepare

while [[ $# > 0 ]]
do
key="$1"

case $key in
    -h|--help)
    echo $'-h|--help \n-u|--username <db_username>'
    exit 0
    shift
    ;;
    -u|--username)
    shift
    USERNAME="$1"
    shift
    ;;
esac
shift
done

psql -f ../postgis/tables.sql $DATABASE $USERNAME

echo Copying nodes...
COMMAND="COPY nodes (id,lat,lon,height,objtype,inside,intunnel,onbridge,stoppos,ref,loc) FROM STDIN;"
./todb n |  psql -c "$COMMAND" $DATABASE $USERNAME

echo Copying ways...
COMMAND="COPY ways (id,area,type,bridge,tunnel) FROM STDIN;"
./todb w | psql -c "$COMMAND" $DATABASE $USERNAME

echo Copying way refs...
COMMAND="COPY ways_refs (id,ref,ord) FROM STDIN;"
./todb v | psql -c "$COMMAND" $DATABASE $USERNAME

echo Copying multipolygons...
COMMAND="COPY multipols (id,objtype) FROM STDIN;"
./todb m | psql -c "$COMMAND" $DATABASE $USERNAME

echo Copying multipolygon references...
COMMAND="COPY multipols_refs (id,ref,role) FROM STDIN;"
./todb l | psql -c "$COMMAND" $DATABASE $USERNAME

COMMAND="ANALYZE nodes; ANALYZE ways; ANALYZE ways_refs; ANALYZE multipols; ANALYZE multipols_refs; "
./todb l | psql -c "$COMMAND" $DATABASE $USERNAME

