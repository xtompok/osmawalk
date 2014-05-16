#!/bin/sh
MINLAT=49.941901
MINLON=14.224436
MAXLAT=50.17743
MAXLON=14.706787

MINLAT=50.10
MINLON=14.40
MAXLAT=50.20
MAXLON=14.50

posledni=`curl "http://osm.kyblsoft.cz/archiv/last_dates.txt" | grep gz | cut -d"	" -f2`
echo "Posledni data z $posledni, stahuji..."

curl "http://osm.kyblsoft.cz/archiv/czech_republic-${posledni}.osm.gz" | gunzip > czech_republic.osm

./osmconvert czech_republic.osm -b=$MINLON,$MINLAT,$MAXLON,$MAXLAT >praha.osm
./merge-srtm $MINLAT $MINLON $MAXLAT $MAXLON

