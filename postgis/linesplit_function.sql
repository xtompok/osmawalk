DROP FUNCTION linesplit(integer,BIGINT);

CREATE OR REPLACE FUNCTION linesplit(integer,BIGINT) 
RETURNS SETOF BIGINT AS '
  DECLARE  
	segments INT;
	parts INT;
	way ways%ROWTYPE; 
	refs BIGINT[];
	seg_len DOUBLE PRECISION;
	line geometry;
	pt geometry;
	new_id BIGINT;
  BEGIN
	SELECT * INTO way FROM ways WHERE id = $2;
	SELECT array(SELECT ref FROM ways_refs WHERE id = $2) INTO refs;
  	RETURN NEXT refs[1];
	SELECT MAX(ord) FROM ways_refs INTO segments WHERE id = $2;
	FOR i IN 1 .. segments LOOP
		seg_len =  ST_Distance(n1.loc,n2.loc) FROM nodes AS n1, nodes AS n2 WHERE n1.id = refs[i] AND n2.id = refs[i+1];
		IF seg_len > $1 THEN
			line =  ST_MakeLine(n1.loc,n2.loc) FROM nodes AS n1, nodes AS n2 WHERE n1.id = refs[i] AND n2.id = refs[i+1];
			FOR j IN 1 .. seg_len/$1 LOOP
				pt = ST_LineInterpolatePoint(line,LEAST(1.0*j*$1 / seg_len,1));
				SELECT MAX(id)+1 FROM nodes INTO new_id;
				INSERT INTO nodes(id,lon,lat,height,loc) VALUES (new_id,ST_X(pt),ST_Y(pt),ST_Z(pt),pt);
				RETURN NEXT new_id;
			END LOOP;
			RETURN NEXT refs[i+1];
		ELSE
			RETURN NEXT refs[i+1];
		END IF;
	END LOOP;
	RETURN;
  END;
' LANGUAGE 'plpgsql';

