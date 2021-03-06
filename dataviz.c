#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "dataviz.h"

int main( int argc, char *argv[] ) {
	if( argc != 2 ) {
		printf( "Usage: dataviz file\n" );
		return 0;
	}

	return read_file( argv[1] );
}

int get_dimensions( struct tag *_tag, int *_w, int *_h, int *_b ) {
	const char delimiter = ';';

	float a_series[12];
		a_series[0] = 46.8;
		a_series[1] = 33.1;
		a_series[2] = 23.4;
		a_series[3] = 16.5;
		a_series[4] = 11.7;
		a_series[5] = 8.3;
		a_series[6] = 5.8;
		a_series[7] = 4.1;
		a_series[8] = 2.9;
		a_series[9] = 2.0;
		a_series[10] = 1.5;
		a_series[11] = 1.0;

	struct tag *container = _tag;

	if( container->val[0] == 'A' ) {
		float w_in_inches = 0, h_in_inches = 0;
		int n = atoi( container->val+1 ); if( n < 0 || n > 10 ) return -1;
		char *tmp; if( !(tmp = strchr( container->val+1, delimiter )) ) return -1;
		if( tmp[1] == 'h' ) {
			w_in_inches = a_series[n];
			h_in_inches = a_series[n+1];
		} else if( tmp[1] == 'v' ) {
			w_in_inches = a_series[n+1];
			h_in_inches = a_series[n];
		} else return -1;
		if( !(tmp = strchr( tmp+1, delimiter )) ) return -1;
		*_w = w_in_inches*atoi( tmp+1 );
		*_h = h_in_inches*atoi( tmp+1 );
	} else {
		*_w = atoi( container->val );
		char *tmp; if( !(tmp = strchr( container->val, delimiter )) ) return -1;
		*_h = atoi( tmp+1 );
	}
	if( container->attr ) *_b = atoi( container->attr );

	return 0;
}

int sector( int _id, char *_id_str, float _cx, float _cy, float _r, float _angle1, float _angle2, FILE *_fp ) {
	float asx, asy, aex, aey; // arc start x and y, arc end x and y
	asx = _r*cos( _angle1 );
	asy = _r*sin( _angle1 );
	aex = _r*cos( _angle2 )-asx;
	aey = _r*sin( _angle2 )-asy;

	char sector_size[4];
	if( abs( _angle1-_angle2 ) > PI )
		strcpy( sector_size, "1 1" );
	else
		strcpy( sector_size, "0 1" );

	fprintf( _fp, "<path id='%i' class='sector sector-%i %s' d='M %f %f l %f %f a %f %f 0 %s %f %f z' />\n", _id, _id, _id_str, _cx, _cy, asx, asy, _r, _r, sector_size, aex, aey );

	return 0;
}

int arrow( float _x1, float _y1, float _x2, float _y2, FILE *_fp ) {
	fprintf( _fp, "<line x1='%f' y1='%f' x2='%f' y2='%f' />\n", _x1, _y1, _x2, _y2 );

	float angle = atan2( (_y2-_y1), (_x2-_x1) );

	float hook1_x = _x2 - cos( angle + PI/4 )*5;
	float hook1_y = _y2 - sin( angle + PI/4 )*5;
	float hook2_x = _x2 - cos( angle - PI/4 )*5;
	float hook2_y = _y2 - sin( angle - PI/4 )*5;
	fprintf( _fp, "<line x1='%f' y1='%f' x2='%f' y2='%f' />\n", _x2, _y2, hook1_x, hook1_y );
	fprintf( _fp, "<line x1='%f' y1='%f' x2='%f' y2='%f' />\n", _x2, _y2, hook2_x, hook2_y );

	return 0;
}

int pie_chart( struct tag *_taglist ) {
	FILE *fp = NULL;

	struct tag *container = _taglist, *style_tag = 0;
	int parent_id = container->id;

	int w = 0, h = 0, b = 0;

	float values_summed = 0;
	while( (container = container->next) ) {
		if( container->parent && container->parent->id == parent_id ) {
			if( strcmp( container->class, "fname" ) == 0 )
				fp = fopen( container->val, "w" );
			if( strcmp( container->class, "style" ) == 0 )
				style_tag = container;
			if( strcmp( container->class, "size" ) == 0 )
				if( get_dimensions( container, &w, &h, &b ) == -1 ) return -1;
			if( strcmp( container->class, "val" ) == 0 )
				values_summed += atof( container->val );
		}
	}

	if( w == 0 || h == 0 ) return -1;

	if( !fp ) fp = stdout;

	fprintf( fp, "<?xml version='1.0' ?>\n" );
	if( style_tag )
		fprintf( fp, "<?xml-stylesheet type='text/css' href='%s' ?>\n", style_tag->val );
	fprintf( fp, "<svg xmlns='http://www.w3.org/2000/svg' width='%i' height='%i'>\n", w+2*b, h+2*b );

	if( w < h ) h = w; w = h;

	container = _taglist;

	int id = 0;
	float angle1 = 0, angle2 = 0;
	while( (container = container->next) ) {
		if( container->parent && container->parent->id == parent_id && strcmp( container->class, "val" ) == 0 ) {
			angle2 += atof( container->val )/values_summed*FULL_ANGLE;
			sector( id, (container->attr ? container->attr : ""), w/2+b, h/2+b, w/2, angle1, angle2, fp );
			angle1 = angle2;
			id++;
		}
	}

	fprintf( fp, "</svg>\n" );

	fclose( fp );

	return 0;
}

int line_graph( struct tag *_taglist ) {
	const char delimiter = ';';

	FILE *fp = NULL;

	struct tag *container = _taglist, *style_tag = 0;
	int parent_id = container->id;

	int w = 0, h = 0, b = 0;
	float reso_x = 0, reso_y = 0;
	float min_x = 0, min_y = 0, max_x = 0, max_y = 0;
	while( (container = container->next) ) {
		if( container->parent && container->parent->id == parent_id ) {
			if( strcmp( container->class, "fname" ) == 0 )
				fp = fopen( container->val, "w" );
			if( strcmp( container->class, "style" ) == 0 )
				style_tag = container;
			if( strcmp( container->class, "size" ) == 0 )
				if( get_dimensions( container, &w, &h, &b ) == -1 ) return -1;
			if( strcmp( container->class, "reso" ) == 0 ) {
				reso_x = atof( container->val );
				char *tmp; if( !(tmp = strchr( container->val, delimiter )) ) return -1;
				reso_y = atof( tmp+1 );
			}
			if( strcmp( container->class, "val" ) == 0 ) {
				float val = atof( container->val );
				if( ( min_x == 0 && val > 0 ) || val < min_x ) min_x = val;
				if( val > max_x ) max_x = val;
				val = atof( strchr( container->val, delimiter )+1 );
				if( ( min_y == 0 && val > 0 ) || val < min_y ) min_y = val;
				if( val > max_y ) max_y = val;
			}
		}
	}

	if( w == 0 || h == 0 ) return -1;
	max_x += reso_x/2;
	max_y += reso_y/2;
	reso_x = reso_x/max_x*w;
	reso_y = reso_y/max_y*h;

	if( !fp ) fp = stdout;

	fprintf( fp, "<?xml version='1.0' ?>\n" );
	if( style_tag )
		fprintf( fp, "<?xml-stylesheet type='text/css' href='%s' ?>\n", style_tag->val );
	fprintf( fp, "<svg xmlns='http://www.w3.org/2000/svg' width='%i' height='%i'>\n", w+2*b, h+2*b );

	float i;
	arrow( b, h+b, w+b, h+b, fp );
	if( reso_x > 0 ) for( i = b+reso_x; i < w+b; i += reso_x )
		fprintf( fp, "<line x1='%f' y1='%i' x2='%f' y2='%i' />\n", i, h+b-5, i, h+b+5 );
	arrow( b, h+b, b, b, fp );
	if( reso_y > 0 ) for( i = h+b-reso_y; i > 0; i -= reso_y )
		fprintf( fp, "<line x1='%i' y1='%f' x2='%i' y2='%f' />\n", b-5, i, b+5, i );

	fprintf( fp, "<polyline class='curve' points='" );

	container = _taglist;

	while( (container = container->next) ) {
		if( container->parent && container->parent->id == parent_id && strcmp( container->class, "val" ) == 0 ) {
			char *tmp;
			float x = atof( container->val )/max_x*w+b;
			if( !(tmp = strchr( container->val, delimiter )) ) return -1;
			float y = h+b-atof( tmp+1 )/max_y*h;
			fprintf( fp, "%f %f ", x, y );
		}
	}

	fprintf( fp, "' />\n</svg>\n" );

	fclose( fp );

	return 0;
}

int bar_chart( struct tag *_taglist ) {
	const char delimiter = ';';

	FILE *fp = NULL;

	struct tag *container = _taglist, *style_tag = 0;
	int parent_id = container->id;

	int number_of_values = 0;
	int w = 0, h = 0, b = 0;
	float reso_x = 0, reso_y = 0;
	float min = 0, max = 0;
	while( (container = container->next) ) {
		if( container->parent && container->parent->id == parent_id ) {
			if( strcmp( container->class, "fname" ) == 0 )
				fp = fopen( container->val, "w" );
			if( strcmp( container->class, "style" ) == 0 )
				style_tag = container;
			if( strcmp( container->class, "size" ) == 0 )
				if( get_dimensions( container, &w, &h, &b ) == -1 ) return -1;
			if( strcmp( container->class, "reso" ) == 0 ) {
				reso_x = atof( container->val );
				char *tmp; if( !(tmp = strchr( container->val, delimiter )) ) return -1;
				reso_y = atof( tmp+1 );
			}
			if( strcmp( container->class, "val" ) == 0 ) {
				float val = atof( container->val );
				if( ( min == 0 && val > 0 ) || val < min ) min = val;
				if( val > max ) max = val;
				number_of_values++;
			}
		}
	}

	if( w == 0 || h == 0 ) return -1;
	max += reso_y/2;
	reso_x = reso_x;
	reso_y = reso_y/max*h;

	if( !fp ) fp = stdout;

	fprintf( fp, "<?xml version='1.0' ?>\n<svg xmlns='http://www.w3.org/2000/svg' width='%i' height='%i'>\n", w+2*b, h+2*b );
	// FIXME (and the other style thingies). Maybe have styles as tag attributes (both, in svg and in dataviz files); though that'd require
	// that there were also tags (in dataviz files) for the coordinate axes.
	if( style_tag ) {
		FILE *stylefp = fopen( style_tag->val, "r" );
		fprintf( fp, "<style>\n" );
		int c; while( (c = fgetc( stylefp )) != EOF ) fputc( c, fp );
		fprintf( fp, "</style>\n" );
		fclose( stylefp );
	}

	float i;
	arrow( b, h+b, w+b, h+b, fp );
	arrow( b, h+b, b, b, fp );
	if( reso_y > 0 ) for( i = h+b-reso_y; i > 0; i -= reso_y )
		fprintf( fp, "<line x1='%i' y1='%f' x2='%i' y2='%f' />\n", b-5, i, b+5, i );

	container = _taglist;

	int id = 0;
	while( (container = container->next) ) {
		if( container->parent && container->parent->id == parent_id && strcmp( container->class, "val" ) == 0 ) {
			float x, y;
			x = b + (id + (1-reso_x)*0.5)*w/number_of_values;
			y = atof( container->val )/max*h;
			fprintf( fp, "<rect id='%i' class='bar bar-%i %s' x='%f' y='%f' width='%f' height='%f' />\n", id, id, (container->attr ? container->attr : ""), x, h+b-y, reso_x*w/number_of_values, y );
			id++;
		}
	}

	fprintf( fp, "</svg>\n" );

	fclose( fp );

	return 0;
}

int read_file( const char *_file ) {
	struct tag *taglist;
	if( tag_struct_init( &taglist ) == -1 ) return 1;

	FILE *fp = fopen( _file, "r" );
	if( !fp ) {
		printf( "Couldn't open file.\n" );
		return 2;
	}

	if( parse( fp, 0, &taglist ) == -1 ) {
		fclose( fp );
		tag_struct_free( taglist );
		return 3;
	}

	fclose( fp );

	struct tag *container = taglist;
	do {
		if( !container->class || strcmp( container->class, "object" ) != 0 || !container->attr ) continue;

		if( strcmp( container->attr, "pie" ) == 0 )
			pie_chart( container );
		if( strcmp( container->attr, "curve" ) == 0 )
			if( line_graph( container ) == -1 )
				return 4;
		if( strcmp( container->attr, "bar" ) == 0 )
			bar_chart( container );
	} while( (container = container->next) );

	tag_struct_free( taglist );

	return 0;
}
