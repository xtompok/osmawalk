#!/bin/sh
MINLAT=49.941901
MINLON=14.224436
MAXLAT=50.17743
MAXLON=14.706787

#50.10694/14.44523
#50.10161/14.45732
#MINLAT=50.08
#MINLON=14.42
#MAXLAT=50.12
#MAXLON=14.457

# Czech Republic
#MINLAT=48.5
#MINLON=12.1
#MAXLAT=51.1
#MAXLON=18.9


if [ $# = 0 ]
then
	posledni=`curl "http://osm.kyblsoft.cz/archiv/last_dates.txt" | grep gz | cut -d"	" -f2`
	echo "Posledni data z $posledni, stahuji..."

	curl "http://osm.kyblsoft.cz/archiv/czech_republic-${posledni}.osm.gz" | gunzip > czech_republic.osm
fi

./osmconvert czech_republic.osm -b=$MINLON,$MINLAT,$MAXLON,$MAXLAT >praha.osm
./merge-srtm $MINLAT $MINLON $MAXLAT $MAXLON

