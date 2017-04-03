DROP TABLE IF EXISTS splitted_ways;
CREATE TABLE splitted_ways AS
	SELECT st_geometrytype(ls.geom),w.*,ls.partid,ls.geom AS splitgeom, ls.posfrom, ls.posto
	FROM ways AS w, linesplit(30,w.geom) AS ls 
	WHERE type NOT IN (0,30) and tunnel = false and bridge = false AND ST_Length(w.geom) > 60 
;

--CREATE TABLE split_nodes AS
