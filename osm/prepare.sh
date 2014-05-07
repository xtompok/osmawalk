#!/bin/sh
MINLAT=50.101
MINLON=14.40
MAXLAT=50.111
MAXLON=14.44

posledni=`curl "http://osm.kyblsoft.cz/archiv/last_dates.txt" | grep gz | cut -d"	" -f2`
echo "Posledni data z $posledni, stahuji..."

curl "http://osm.kyblsoft.cz/archiv/czech_republic-${posledni}.osm.gz" | gunzip > czech_republic.osm

./osmconvert czech_republic.osm -b=$MINLON,$MINLAT,$MAXLON,$MAXLAT >praha.osm
#./osmconvert czech_republic.osm -b=14.224436,49.941901,14.706787,50.17743 >praha.osm

