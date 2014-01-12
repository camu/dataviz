#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

/*int main( int argc, char *argv[] ) {
	struct tag *tags;
	tag_struct_init( &tags );

	FILE *fp = fopen( "example", "r" );

	parse( fp, 0, &tags );

	fclose( fp );

	struct tag *container = tags;
	do {
		if( container->attr[0] != 0 )
			printf( "<%s %s>%s</%s>\n", container->class, container->attr, container->val, container->class );
		else
			printf( "<%s>%s</%s>\n", container->class, container->val, container->class );
	} while( (container = container->next) );

	tag_struct_free( tags );

	return 0;
}*/

int id = 0;

int tag_struct_init( struct tag **_tags ) {
	*_tags = malloc( sizeof(struct tag) );
	(*_tags)->id = id; id++;
	(*_tags)->class = 0;
	(*_tags)->attr = 0;
	(*_tags)->val = 0;
	(*_tags)->parent = 0;
	(*_tags)->next = 0;

	return 0;
}

int tag_struct_set_class( const char * _class, int _len, struct tag **_tags ) {
	(*_tags)->class = malloc( _len+1 );
	strcpy( (*_tags)->class, _class );

	return 0;
}

int tag_struct_set_attr( const char * _attr, int _len, struct tag **_tags ) {
	(*_tags)->attr = malloc( _len+1 );
	strcpy( (*_tags)->attr, _attr );

	return 0;
}

int tag_struct_set_val( const char * _val, int _len, struct tag **_tags ) {
	(*_tags)->val = malloc( _len+1 );
	strcpy( (*_tags)->val, _val );

	return 0;
}

struct tag *tag_struct_push_back( struct tag **_tags ) {
	struct tag *last = *_tags;
	while( last->next ) last = last->next;
	last->next = malloc( sizeof(struct tag) );
	tag_struct_init( &last->next );

	return last->next;
}

int tag_struct_free( struct tag *_tags ) {
	if( _tags->next ) tag_struct_free( _tags->next );
	free( _tags->class );
	free( _tags->attr );
	free( _tags->val );
	free( _tags );

	return 0;
}

char get_tag( FILE *_fp, char **_dest ) {
	*_dest = malloc( PARSERBUFLEN );
	int i; for( i = 0; i < PARSERBUFLEN; i++ ) (*_dest)[i] = 0;
	int len = 0, allocd = PARSERBUFLEN;
	char c = fgetc( _fp );
	do {
		if( c == ' ' || c == '>' ) break;
		(*_dest)[len] = c;
		len++;
		if( len == allocd ) *_dest = realloc( *_dest, allocd += PARSERBUFLEN );
	} while( (c = fgetc( _fp )) != EOF );

	return c;
}

char get_attr( FILE *_fp, char **_dest ) {
	*_dest = malloc( PARSERBUFLEN );
	int i; for( i = 0; i < PARSERBUFLEN; i++ ) (*_dest)[i] = 0;
	int len = 0, allocd = PARSERBUFLEN;
	char c = fgetc( _fp );
	do {
		if( c == '>' ) break;
		(*_dest)[len] = c;
		len++;
		if( len == allocd ) *_dest = realloc( *_dest, allocd += PARSERBUFLEN );
	} while( (c = fgetc( _fp )) != EOF );

	return c;
}

int parse( FILE *_fp, struct tag *_tag, struct tag **_tags ) {
	char *val = malloc( PARSERBUFLEN );
	int len = 0, allocd = PARSERBUFLEN;

	struct tag *container = _tag;

	char c = fgetc( _fp );
	do {
		if( c == '<' ) {
			char *tag = 0;
			c = get_tag( _fp, &tag );
			if( tag[0] != '/' ) {
				struct tag *parent = container;
				container = tag_struct_push_back( _tags );
				tag_struct_set_class( tag, strlen( tag ), &container );
				container->parent = parent;
			} else /* if( strcmp( _tag, tag+1 ) == 0 ) */ {
				tag_struct_set_val( val, strlen( val ), &container );
				return 0;
			}

			if( c == ' ' ) {
				char *attr = 0;
				c = get_attr( _fp, &attr );
				tag_struct_set_attr( attr, strlen( attr ), &container );
			}

			parse( _fp, container, _tags );
			container = container->parent;
			c = fgetc( _fp );
		}

		val[len] = c; len++;
		if( len == allocd ) val = realloc( val, allocd += PARSERBUFLEN );
	} while( (c = fgetc( _fp )) != EOF );

	return 0;
}
