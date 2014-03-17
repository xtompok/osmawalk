#!/bin/sh

posledni=`curl "http://osm.kyblsoft.cz/archiv/last_dates.txt" | grep bz2 | cut -d"	" -f2`
echo "Posledni data z $posledni, stahnout?"

curl "http://osm.kyblsoft.cz/archiv/czech_republic-$posledni.osm.bz2" | bunzip2 > czech_republic.osm

./osmconvert czech_republic.osm -b=14.4,50.05,14.5,50.10 >praha.osm
#./osmconvert czech_republic.osm -b=14.224436,49.941901,14.706787,50.17743 >praha.osm

