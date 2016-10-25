// Compile with gcc -std=c99 -lm -o merge-srtm merge-srtm.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

int main(int argc, char ** argv){
	if (argc != 5){
		printf("Usage: %s minlat minlon maxlat maxlon\n",argv[0]);
		return 1;
	}
	
	int minlon;
	int minlat;
	int maxlon;
	int maxlat;
	
	minlon = floor(atof(argv[2]));
	minlat = floor(atof(argv[1]));
	maxlon = floor(atof(argv[4]));
	maxlat = floor(atof(argv[3]));

	char name[12];
	FILE * hgt;

	uint16_t * map;
	size_t len;
	len = (2*(maxlon-minlon+1)*1200*(maxlat-minlat+1)*1200);
	map = malloc(len);
	uint16_t * sq;
	sq = malloc(2*1201*1201);

	int rowlen;
	rowlen = 1200*(maxlon-minlon+1);

	
	
	for (int lon=0; lon <= maxlon-minlon; lon++){
		for (int lat=0; lat <= maxlat-minlat; lat++){
			sprintf(name,"N%02dE%03d.hgt",lat+minlat,lon+minlon);
			hgt = fopen(name,"r");	
			if (hgt == NULL){
				printf("File %s not found, exitting, no output produced\n",name);
				return 1;
			}
			printf("Opened %s\n",name);
			fread(sq,1201*1201,2,hgt);
			for (int llon = 0; llon < 1200; llon++){
				for (int llat = 0; llat < 1200; llat++){
					uint16_t height;
					height = sq[(1200-llat)*1201+llon];
					height = (height>>8)|(height<<8);
					if (height == 32768)
						height=map[rowlen*(1200*lat+llat) + 1200*lon+llon-1]; 
					map[rowlen*(1200*lat+llat) + 1200*lon+llon] = height;
				}
			
			}

		}
	}
	
	FILE * OUT;
	OUT = fopen("heights.txt","w");
	fprintf(OUT,"%d %d %d %d\n",minlon, minlat, maxlon, maxlat);

	for (int lat=0; lat<1200*(maxlat-minlat+1);lat++){
		for (int lon=0; lon < 1200*(maxlon-minlon+1); lon++){
			fprintf(OUT,"%d ",map[rowlen*lat + lon]);
		}
		fprintf(OUT,"\n");
	}

	fclose(OUT);

	OUT = fopen("heights.bin","w");
	fwrite(&minlon,1,sizeof(int),OUT);
	fwrite(&minlat,1,sizeof(int),OUT);
	fwrite(&maxlon,1,sizeof(int),OUT);
	fwrite(&maxlat,1,sizeof(int),OUT);
	fwrite(map,len,1,OUT);
	fclose(OUT);

}
