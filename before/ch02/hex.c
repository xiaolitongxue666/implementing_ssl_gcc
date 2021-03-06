#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hex.h"

int hex_decode( const unsigned char *input, unsigned char **decoded )
{
 int i;
 int len;

 if ( strncmp( "0x", input, 2 ) )
 {
   len = strlen( input ) + 1;
   *decoded = malloc( len );
   strcpy( *decoded, input );
   len--;
 }
 else
 {
   len = ( strlen( input ) >> 1 ) - 1;
   *decoded = malloc( len );
   for ( i = 2; i < strlen( input ); i += 2 )
   {
     (*decoded)[ ( ( i / 2 ) - 1 ) ] =
       ( ( ( input[ i ] <= '9' ) ? input[ i ] - '0' :
( ( tolower( input[ i ] ) ) - 'a' + 10 ) ) << 4 ) |
       ( ( input[ i + 1 ] <= '9' ) ? input[ i + 1 ] - '0' :
( ( tolower( input[ i + 1 ] ) ) - 'a' + 10 ) );
   }
 }

 return len;
}

void show_hex( const unsigned char *array, int length )
{
 while ( length-- )
 {
   printf( "%.02x", *array++ );
 }
 printf( "\n" );
}
