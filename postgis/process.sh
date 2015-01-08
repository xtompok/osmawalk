#!/bin/sh

step(){
	echo
	echo $1
	(time psql -v VERBOSITY=verbose -e -f "${1}.sql" osmwalk-prepare jethro) 2>&1
}

step duplicate_nodes
step ways_geom
step mp_geom
step walk_in_nodes
step barriers
step bbox-function
step direct_candidates2
step ok_candidates2
step direct_filter
./export-direct.sh
./export-nodes.sh
./export-ways.sh
