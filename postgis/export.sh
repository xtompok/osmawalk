#!/bin/sh
USER=jethro
DATABASE=osmwalk-prepare

COMMAND="COPY direct(id1,id2) TO STDOUT DELIMITER ';' CSV HEADER;"
psql -c "$COMMAND" $DATABASE $USERNAME >direct.csv  

COMMAND="COPY ( \
	SELECT n.id AS id, n.lat AS lat, n.lon AS lon, n.height AS height\
	FROM nodes AS n\
	INNER JOIN walk_nodes AS wn ON n.id = wn.id\
	) TO STDOUT WITH DELIMITER ';' CSV  HEADER;"
psql -c "$COMMAND" $DATABASE $USERNAME >nodes.csv  

COMMAND="\
COPY ( \
	SELECT w.id AS id, w.type AS type, wr1.ref AS from, wr2.ref AS to\
	FROM ways AS w\
	INNER JOIN ways_refs AS wr1 ON w.id = wr1.id \
	INNER JOIN ways_refs AS wr2 ON w.id = wr2.id\
	WHERE w.type != 0 AND w.type != 30 AND (
		(wr1.ord-wr2.ord) = 1 OR (wr2.ord-wr1.ord) = 1
	)
) TO STDOUT WITH DELIMITER ';' CSV HEADER;"
psql -c "$COMMAND" $DATABASE $USERNAME >ways.csv  
