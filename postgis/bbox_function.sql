WITH sq AS (
	SELECT  MIN(lon) AS minlon, MAX(lon) AS maxlon,
		MIN(lat) AS minlat, MAX(lat) AS maxlat,
		CEIL((MAX(lon)-MIN(lon))/100) AS lonparts,
		CEIL((MAX(lat)-MIN(lat))/100) AS latparts
	FROM nodes
) 
UPDATE nodes 
SET square00 = FLOOR((lat-sq.minlat)/100)*lonparts+FLOOR((lon-sq.minlon)/100),
	square11 = FLOOR((lat-sq.minlat+50)/100)*lonparts+FLOOR((lon-sq.minlon+50)/100),
	square01 = FLOOR((lat-sq.minlat+50)/100)*lonparts+FLOOR((lon-sq.minlon)/100),
	square10 = FLOOR((lat-sq.minlat)/100)*lonparts+FLOOR((lon-sq.minlon+50)/100)
 FROM sq;

CREATE INDEX ON nodes(square00);
CREATE INDEX ON nodes(square10);
CREATE INDEX ON nodes(square01);
CREATE INDEX ON nodes(square11);
