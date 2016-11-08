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

step duplicate_nodes
step linesplit_function2
step split_ways2
step ways_geom
step mp_geom
step barriers
step walk_in_nodes
step bbox-function2
step direct_candidates5
step ok_candidates2
step direct_filter
./export.sh
