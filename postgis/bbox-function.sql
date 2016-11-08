SELECT MIN(lon),MIN(lat),MAX(lon),MAX(lat)
FROM nodes;

DROP TYPE ow_bbox CASCADE;
CREATE TYPE ow_bbox AS (id INT, minx INT, miny INT, maxx INT, maxy INT, geom geometry);

CREATE OR REPLACE FUNCTION make_boxes(integer, integer) 
RETURNS SETOF ow_bbox AS '
  DECLARE  
	minx INT; 
	miny INT; 
	maxx INT; 
	maxy INT; 
	dx INT; 
	dy INT;
	line ow_bbox ;
	memminy INT;
	loopx INT;
	loopy INT;
  BEGIN
    SELECT INTO minx MIN(lon) FROM nodes;
    SELECT INTO miny MIN(lat) FROM nodes;
    SELECT INTO maxx MAX(lon) FROM nodes;
    SELECT INTO maxy MAX(lat) FROM nodes;
    dx := $1;
    dy := $1;
    memminy := miny;

    loopx = ((maxx-minx)/dx);
    loopy = ((maxy-miny)/dy);
      
    FOR i IN 1..loopx LOOP
      FOR j IN 1..loopy LOOP
	line.minx = minx+$2;
	line.maxx = minx+dx+$2;
	line.miny = miny+$2;
	line.maxy = miny+dy+$2;
	line.geom = ST_MakeBox2D(ST_Point(line.minx,line.miny),ST_Point(line.maxx,line.maxy));
	line.id = i*loopx + j;
	miny := miny + dy;
	RETURN NEXT line;
      END LOOP;
      minx := minx + dx;
      miny := memminy;
    END LOOP;
    RETURN;
  END;
' LANGUAGE 'plpgsql';

DROP TABLE IF EXISTS bboxes_100_0;
CREATE TABLE bboxes_100_0 AS
	SELECT * FROM make_boxes(100,0);

DROP TABLE IF EXISTS bboxes_100_50;
CREATE TABLE bboxes_100_50 AS
	SELECT * FROM make_boxes(100,50);

CREATE INDEX ON bboxes_100_50 USING GIST(geom);
CREATE INDEX ON bboxes_100_0 USING GIST(geom);

UPDATE nodes SET square1=bb.id
FROM bboxes_100_0 AS bb INNER JOIN nodes AS n ON n.loc && bb.geom;

