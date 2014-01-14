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

	// FIXME. somewhat work's if the arrow's strictly horizontal or vertical; otherwise not.
	float hook1_x = cos( atan( abs( _x2-_x1 ) )+PI/4 )*5;
	float hook1_y = sin( atan( abs( _x2-_x1 ) )+PI/4 )*5;
	float hook2_x = cos( atan( abs( _x2-_x1 ) )-PI*5/4 )*5;
	float hook2_y = sin( atan( abs( _x2-_x1 ) )-PI*5/4 )*5;
	fprintf( _fp, "<line x1='%f' y1='%f' x2='%f' y2='%f' />\n", _x2, _y2, _x2+hook1_x, _y2+hook1_y );
	fprintf( _fp, "<line x1='%f' y1='%f' x2='%f' y2='%f' />\n", _x2, _y2, _x2+hook2_x, _y2+hook2_y );

	return 0;
}

int pie_chart( struct tag *_taglist ) {
	FILE *fp = NULL;

	struct tag *container = _taglist, *style_tag = 0;
	int parent_id = container->id;

	float values_summed = 0;
	while( (container = container->next) ) {
		if( container->parent && container->parent->id == parent_id ) {
			if( strcmp( container->class, "val" ) == 0 )
				values_summed += atof( container->val );
			if( strcmp( container->class, "fname" ) == 0 )
				fp = fopen( container->val, "w" );
			if( strcmp( container->class, "style" ) == 0 )
				style_tag = container;
		}
	}

	if( !fp ) fp = stdout;

	fprintf( fp, "<?xml version='1.0' ?>\n" );
	if( style_tag )
		fprintf( fp, "<?xml-stylesheet type='text/css' href='%s' ?>\n", style_tag->val );
	fprintf( fp, "<svg xmlns='http://www.w3.org/2000/svg' width='1000' height='1000'>\n" );

	container = _taglist;

	int id = 0;
	float angle1 = 0, angle2 = 0;
	while( (container = container->next) ) {
		if( container->parent && container->parent->id == parent_id && strcmp( container->class, "val" ) == 0 ) {
			angle2 += atof( container->val )/values_summed*FULL_ANGLE;
			sector( id, (container->attr ? container->attr : ""), 200, 200, 100, angle1, angle2, fp );
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

	float min_x = 0, min_y = 0, max_x = 0, max_y = 0;
	while( (container = container->next) ) {
		if( container->parent && container->parent->id == parent_id ) {
			if( strcmp( container->class, "fname" ) == 0 )
				fp = fopen( container->val, "w" );
			if( strcmp( container->class, "style" ) == 0 )
				style_tag = container;
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

	if( !fp ) fp = stdout;

	fprintf( fp, "<?xml version='1.0' ?>\n" );
	if( style_tag )
		fprintf( fp, "<?xml-stylesheet type='text/css' href='%s' ?>\n", style_tag->val );
	fprintf( fp, "<svg xmlns='http://www.w3.org/2000/svg' width='1000' height='1000'>\n" );

	arrow( 10, 510, 510, 510, fp );
	arrow( 10, 510, 10, 10, fp );
	fprintf( fp, "<polyline class='curve' points='" );

	container = _taglist;

	while( (container = container->next) ) {
		if( container->parent && container->parent->id == parent_id && strcmp( container->class, "val" ) == 0 ) {
			char *tmp;
			float x = atof( container->val )/max_x*500+10;
			if( !(tmp = strchr( container->val, delimiter )) ) return -1;
			float y = 510-atof( tmp+1 )/max_y*500;
			fprintf( fp, "%f %f ", x, y );
		}
	}

	fprintf( fp, "' />\n</svg>\n" );

	fclose( fp );

	return 0;
}

int bar_chart( struct tag *_taglist ) {
	FILE *fp = NULL;

	struct tag *container = _taglist, *style_tag = 0;
	int parent_id = container->id;

	float min = 0, max = 0;
	while( (container = container->next) ) {
		if( container->parent && container->parent->id == parent_id ) {
			if( strcmp( container->class, "fname" ) == 0 )
				fp = fopen( container->val, "w" );
			if( strcmp( container->class, "style" ) == 0 )
				style_tag = container;
			if( strcmp( container->class, "val" ) == 0 ) {
				float val = atof( container->val );
				if( ( min == 0 && val > 0 ) || val < min ) min = val;
				if( val > max ) max = val;
			}
		}
	}

	if( !fp ) fp = stdout;

	fprintf( fp, "<?xml version='1.0' ?>\n" );
	if( style_tag )
		fprintf( fp, "<?xml-stylesheet type='text/css' href='%s' ?>\n", style_tag->val );
	fprintf( fp, "<svg xmlns='http://www.w3.org/2000/svg' width='1000' height='1000'>\n" );

	arrow( 10, 510, 510, 510, fp );
	arrow( 10, 510, 10, 10, fp );

	container = _taglist;

	int id = 0;
	while( (container = container->next) ) {
		if( container->parent && container->parent->id == parent_id && strcmp( container->class, "val" ) == 0 ) {
			float x, y;
			x = (id+1)*80;
			y = 510-atof( container->val )/max*500;
			fprintf( fp, "<line id='%i' class='bar bar-%i %s' x1='%f' y1='%f' x2='%f' y2='%f' />\n", id, id, (container->attr ? container->attr : ""), x, (float)510, x, y );
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
