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

step(){
	echo
	echo $1
	#(time psql -v VERBOSITY=verbose -e -f "${1}.sql" $DATABASE $USERNAME) 2>&1
	(time psql  -f "${1}.sql" $DATABASE $USERNAME) 2>&1
}

./stops.sh

step linesplit_function
step split_ways 
step ways_geom
step mp_geom
step barriers
step walk_in_nodes
step bbox_function
step direct_candidates
step ok_candidates
step direct_filter
step stops
step stops_bbox
step stops_direct_cand
step stops_direct_ok
./export.sh
