#!/bin/sh

USERNAME=jethro
DATABASE=osmwalk-prepare

psql -c "DROP TABLE IF EXISTS nodes,ways,ways_refs,multipols,multipols_refs" $DATABASE $USERNAME
psql -f ../postgis/tables.sql $DATABASE $USERNAME

echo Copying nodes...
COMMAND="COPY nodes (id,lat,lon,height,objtype,inside,intunnel,onbridge,loc) FROM STDIN;"
./todb n |  psql -c "$COMMAND" $DATABASE $USERNAME

echo Copying ways...
COMMAND="COPY ways (id,area,barrier,type,bridge,tunnel,wayidx) FROM STDIN;"
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
