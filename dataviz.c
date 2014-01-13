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

int pie_chart( struct tag *_taglist ) {
	FILE *fp = NULL;

	struct tag *container = _taglist, *style_tag = 0;
	int parent_id = container->id;

	float values_summed = 0;
	while( (container = container->next) ) {
		if( container->parent->id == parent_id && strcmp( container->class, "val" ) == 0 )
			values_summed += atof( container->val );
		if( strcmp( container->class, "fname" ) == 0 )
			fp = fopen( container->val, "w" );
		if( strcmp( container->class, "style" ) == 0 )
			style_tag = container;
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
		if( container->parent->id == parent_id && strcmp( container->class, "val" ) == 0 ) {
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
		return 1;
	}

	fclose( fp );

	struct tag *container = taglist;
	do {
		if( !container->class || strcmp( container->class, "object" ) != 0 || !container->attr ) continue;

		if( strcmp( container->attr, "pie" ) == 0 )
			pie_chart( container );
	} while( (container = container->next) );

	tag_struct_free( taglist );

	return 0;
}
