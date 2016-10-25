#!/bin/sh

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
psql -c "$COMMAND" osmwalk-prepare jethro >ways.csv  


