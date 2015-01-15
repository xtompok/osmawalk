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
	(time psql -v VERBOSITY=verbose -e -f "${1}.sql" $DATABASE $USERNAME) 2>&1
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
