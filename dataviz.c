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

int sector( float _cx, float _cy, float _r, float _angle1, float _angle2, char *_options, FILE *_fp ) {
	float asx, asy, aex, aey; // arc start x and y, arc end x and y
	asx = _r*cos( _angle1 );
	asy = _r*sin( _angle1 );
	aex = _r*cos( _angle2 )-asx;
	aey = _r*sin( _angle2 )-asy;

	fprintf( _fp, "<path d='M %f %f l %f %f a %f %f 0 %i %i %f %f z' %s />\n", _cx, _cy, asx, asy, _r, _r, 0, 1, aex, aey, _options );

	return 0;
}

int pie_chart( struct tag *_taglist ) {
	FILE *fp;

	struct tag *container = _taglist;
	int parent_id = container->id;

	float values_summed = 0;
	while( (container = container->next) ) {
		if( container->parent->id == parent_id && strcmp( container->class, "val" ) == 0 )
			values_summed += atof( container->val );
		if( strcmp( container->class, "fname" ) == 0 )
			fp = fopen( container->val, "w" );
	}

	if( !fp ) fp = stdout;

	fprintf( fp, "<?xml version='1.0' ?>\n<svg xmlns='http://www.w3.org/2000/svg' width='1000' height='1000'>\n" );
	//fprintf( fp, "<?xml-stylesheet type='text/css' href='style.css'?>\n" );

	container = _taglist;

	float angle1 = 0, angle2 = 0;
	while( (container = container->next) ) {
		if( container->parent->id == parent_id && strcmp( container->class, "val" ) == 0 ) {
			angle2 += atof( container->val )/values_summed*FULL_ANGLE;
			sector( 200, 200, 100, angle1, angle2, "", fp );
			angle1 = angle2;
		}
	}

	fprintf( fp, "</svg>\n" );

	fclose( fp );

	return 0;
}

int read_file( const char *_file ) {
	struct tag *taglist;
	tag_struct_init( &taglist );

	FILE *fp = fopen( _file, "r" );
	if( !fp ) {
		printf( "Couldn't open file.\n" );
		return 1;
	}

	parse( fp, 0, &taglist );

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
