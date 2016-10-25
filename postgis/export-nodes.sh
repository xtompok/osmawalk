#!/bin/sh

COMMAND="COPY ( \
	SELECT n.id AS id, n.lat AS lat, n.lon AS lon, n.height AS height\
	FROM nodes AS n\
	INNER JOIN walk_nodes AS wn ON n.id = wn.id\
	) TO STDOUT WITH DELIMITER ';' CSV  HEADER;"
psql -c "$COMMAND" osmwalk-prepare jethro >nodes.csv  


