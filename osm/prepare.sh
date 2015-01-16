#!/bin/sh

#50.10694/14.44523
#50.10161/14.45732
#MINLAT=50.08
#MINLON=14.42
#MAXLAT=50.12
#MAXLON=14.457

# Prague
MINLAT=50.0
MINLON=14.5
MAXLAT=50.17743
MAXLON=14.706787

# Czech Republic
# MINLAT=48.5
# MINLON=12.1
# MAXLAT=51.1
# MAXLON=18.9

DOWNLOAD_OSM=1
DOWNLOAD_HGT=1
CUT=1
MERGE=1

for i
do
	case $i in
		--noosm)
			DOWNLOAD_OSM=0
			;;
		--nohgt)
			DOWNLOAD_HGT=0
			;;
		--nocut)
			CUT=0
			;;
		--nomerge)
			MERGE=0
			;;
		*)
			echo "$0 [--noosm] [--nohgt] [--nocut] [--nomerge]"
			exit
			;;
	esac
	shift
done


if [ $DOWNLOAD_OSM = 1 ]
then
	echo "Downloading OSM data for Czech Republic"
	posledni=`curl "http://osm.kyblsoft.cz/archiv/last_dates.txt" | grep gz | cut -d"	" -f2`
	echo "Posledni data z $posledni, stahuji..."

	curl "http://osm.kyblsoft.cz/archiv/czech_republic-${posledni}.osm.gz" | gunzip > czech_republic.osm
fi

if [ $DOWNLOAD_HGT = 1 ]
then
	echo "Downloading SRTM data for Czech Republic"
    wget --accept-regex "http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Eurasia/N(48|49|50|51)E01[12345678].hgt.zip" -rl 0 -nH --cut-dirs=20 http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Eurasia/
    unzip '*.hgt.zip' && rm *.hgt.zip
fi

if [ $CUT = 1 ]
then
	echo "Cutting rectangle from OSM data"
	./osmconvert czech_republic.osm -b=$MINLON,$MINLAT,$MAXLON,$MAXLAT >praha.osm
else
	cp czech_republic.osm praha.osm
fi

if [ $MERGE = 1 ]
then
	echo "Merging SRTM files"
	./merge-srtm $MINLAT $MINLON $MAXLAT $MAXLON
fi
