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

echo Exporting direct...
COMMAND="COPY direct(id1,id2) TO STDOUT DELIMITER ';' CSV HEADER;"
psql -c "$COMMAND" $DATABASE $USERNAME >../data/direct.csv  

echo Exporting nodes...
COMMAND="COPY ( \
	SELECT n.id AS id, n.lat AS lat, n.lon AS lon, n.height AS height\
	FROM nodes AS n\
	INNER JOIN walk_nodes AS wn ON n.id = wn.id\
	) TO STDOUT WITH DELIMITER ';' CSV  HEADER;"
psql -c "$COMMAND" $DATABASE $USERNAME >../data/nodes.csv  

echo Exporting ways...
COMMAND="\
COPY ( \
	SELECT w.id AS id, w.type AS type, wr1.ref AS from, wr2.ref AS to\
	FROM ways AS w\
	INNER JOIN ways_refs AS wr1 ON w.id = wr1.id \
	INNER JOIN ways_refs AS wr2 ON w.id = wr2.id\
	WHERE w.type != 0 AND w.type != 30 AND w.type!= 256 AND (
		(wr1.ord-wr2.ord) = 1 OR (wr2.ord-wr1.ord) = 1
	)
) TO STDOUT WITH DELIMITER ';' CSV HEADER;"
psql -c "$COMMAND" $DATABASE $USERNAME >../data/ways.csv  

echo Exporting stops...
COMMAND="COPY stops(id,stop_id,lat,lon) TO STDOUT WITH DELIMITER ';' CSV HEADER;"
psql -c "$COMMAND" $DATABASE $USERNAME >../data/stops.csv

echo Exporting stops ways...
COMMAND="COPY stops_direct_ok(sid,nid) TO STDOUT DELIMITER ';' CSV HEADER;"
psql -c "$COMMAND" $DATABASE $USERNAME >../data/stops-direct.csv
