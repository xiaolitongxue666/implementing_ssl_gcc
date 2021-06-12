#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "des.h"
#include "hex.h"

// This does not return a 1 for a 1 bit; it just return none-zero 
#define GET_BIT( array, bit ) \
    ( array[ ( int ) ( bit / 8 ) ] & ( 0x80 >> ( bit % 8 ) ) )

#define SET_BIT( array, bit ) \
    ( array[ (int) (bit / 8 ) ] |= (0x80 >> (bit % 8 ) ) )

#define CLEAR_BIT( array, bit ) \
    ( array[ (int) (bit / 8 ) ] &= ~(0x80 >> (bit % 8) ) )

static const int ip_table[] = {
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6,
    64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9,  1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7 };

static const int fp_table[] = {
    40, 8, 48, 16, 56, 24, 64, 32,
    39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30,
    37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28,
    35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26,
    33, 1, 41,  9, 49, 17, 57, 25 };

static const int pc1_table[] = {
    57, 49, 41, 33, 25, 17,  9, 1,
    58, 50, 42, 34, 26, 18, 10, 2,
    59, 51, 43, 35, 27, 19, 11, 3,
    60, 52, 44, 36,
    63, 55, 47, 39, 31, 23, 15, 7,
    62, 54, 46, 38, 30, 22, 14, 6,
    61, 53, 45, 37, 29, 21, 13, 5,
    28, 20, 12,  4 };

static const int pc2_table[] = { 
    14, 17, 11, 24,  1,  5,
    3, 28, 15,  6, 21, 10,
    23, 19, 12,  4, 26,  8,
    16,  7, 27, 20, 13,  2,
    41, 52, 31, 37, 47, 55,
    30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53,
    46, 42, 50, 36, 29, 32 };

static const int expansion_table[] = {
     32,  1,  2,  3,  4,  5,
      4,  5,  6,  7,  8,  9,
      8,  9, 10, 11, 12, 13,
      12, 13, 14, 15, 16, 17,
      16, 17, 18, 19, 20, 21,
      20, 21, 22, 23, 24, 25,
      24, 25, 26, 27, 28, 29,
      28, 29, 30, 31, 32,  1 };

static const int sbox[8][64] = {
{ 14, 0, 4, 15, 13, 7, 1, 4, 2, 14, 15, 2, 11, 13, 8, 1,
   3, 10, 10, 6, 6, 12, 12, 11, 5, 9, 9, 5, 0, 3, 7, 8,
   4, 15, 1, 12, 14, 8, 8, 2, 13, 4, 6, 9, 2, 1, 11, 7,
  15, 5, 12, 11, 9, 3, 7, 14, 3, 10, 10, 0, 5, 6, 0, 13 },
{ 15, 3, 1, 13, 8, 4, 14, 7, 6, 15, 11, 2, 3, 8, 4, 14,
  9, 12, 7, 0, 2, 1, 13, 10, 12, 6, 0, 9, 5, 11, 10, 5,
  0, 13, 14, 8, 7, 10, 11, 1, 10, 3, 4, 15, 13, 4, 1, 2,
  5, 11, 8, 6, 12, 7, 6, 12, 9, 0, 3, 5, 2, 14, 15, 9 },
{ 10, 13, 0, 7, 9, 0, 14, 9, 6, 3, 3, 4, 15, 6, 5, 10,
  1, 2, 13, 8, 12, 5, 7, 14, 11, 12, 4, 11, 2, 15, 8, 1,
 13, 1, 6, 10, 4, 13, 9, 0, 8, 6, 15, 9, 3, 8, 0, 7,
 11, 4, 1, 15, 2, 14, 12, 3, 5, 11, 10, 5, 14, 2, 7, 12 },
{ 7, 13, 13, 8, 14, 11, 3, 5, 0, 6, 6, 15, 9, 0, 10, 3,
  1, 4, 2, 7, 8, 2, 5, 12, 11, 1, 12, 10, 4, 14, 15, 9,
 10, 3, 6, 15, 9, 0, 0, 6, 12, 10, 11, 1, 7, 13, 13, 8,
 15, 9, 1, 4, 3, 5, 14, 11, 5, 12, 2, 7, 8, 2, 4, 14 },
{ 2, 14, 12, 11, 4, 2, 1, 12, 7, 4, 10, 7, 11, 13, 6, 1,
  8, 5, 5, 0, 3, 15, 15, 10, 13, 3, 0, 9, 14, 8, 9, 6,
  4, 11, 2, 8, 1, 12, 11, 7, 10, 1, 13, 14, 7, 2, 8, 13,
 15, 6, 9, 15, 12, 0, 5, 9, 6, 10, 3, 4, 0, 5, 14, 3 },
{ 12, 10, 1, 15, 10, 4, 15, 2, 9, 7, 2, 12, 6, 9, 8, 5,
  0, 6, 13, 1, 3, 13, 4, 14, 14, 0, 7, 11, 5, 3, 11, 8,
  9, 4, 14, 3, 15, 2, 5, 12, 2, 9, 8, 5, 12, 15, 3, 10,
  7, 11, 0, 14, 4, 1, 10, 7, 1, 6, 13, 0, 11, 8, 6, 13 },
{ 4, 13, 11, 0, 2, 11, 14, 7, 15, 4, 0, 9, 8, 1, 13, 10,
  3, 14, 12, 3, 9, 5, 7, 12, 5, 2, 10, 15, 6, 8, 1, 6,
  1, 6, 4, 11, 11, 13, 13, 8, 12, 1, 3, 4, 7, 10, 14, 7,
  10, 9, 15, 5, 6, 0, 8, 15, 0, 14, 5, 2, 9, 3, 2, 12 },
{ 13, 1, 2, 15, 8, 13, 4, 8, 6, 10, 15, 3, 11, 7, 1, 4,
 10, 12, 9, 5, 3, 6, 14, 11, 5, 0, 0, 14, 12, 9, 7, 2,
  7, 2, 11, 1, 4, 14, 1, 7, 9, 4, 12, 10, 14, 8, 2, 13,
  0, 15, 6, 12, 10, 9, 13, 0, 15, 3, 3, 5, 5, 6, 8, 11 }
};

static const int p_table[] = { 16,  7, 20, 21,
                              29, 12, 28, 17,
                               1, 15, 23, 26,
                               5, 18, 31, 10,
                                2,  8, 24, 14,
                               32, 27,  3,  9,
                               19, 13, 30,  6,
                               22, 11,  4, 25 };

static const int sbox[8][64] = {
{ 14, 0, 4, 15, 13, 7, 1, 4, 2, 14, 15, 2, 11, 13, 8, 1,
   3, 10, 10, 6, 6, 12, 12, 11, 5, 9, 9, 5, 0, 3, 7, 8,
   4, 15, 1, 12, 14, 8, 8, 2, 13, 4, 6, 9, 2, 1, 11, 7,
  15, 5, 12, 11, 9, 3, 7, 14, 3, 10, 10, 0, 5, 6, 0, 13 },
{ 15, 3, 1, 13, 8, 4, 14, 7, 6, 15, 11, 2, 3, 8, 4, 14,
  9, 12, 7, 0, 2, 1, 13, 10, 12, 6, 0, 9, 5, 11, 10, 5,
  0, 13, 14, 8, 7, 10, 11, 1, 10, 3, 4, 15, 13, 4, 1, 2,
  5, 11, 8, 6, 12, 7, 6, 12, 9, 0, 3, 5, 2, 14, 15, 9 },
{ 10, 13, 0, 7, 9, 0, 14, 9, 6, 3, 3, 4, 15, 6, 5, 10,
  1, 2, 13, 8, 12, 5, 7, 14, 11, 12, 4, 11, 2, 15, 8, 1,
 13, 1, 6, 10, 4, 13, 9, 0, 8, 6, 15, 9, 3, 8, 0, 7,
 11, 4, 1, 15, 2, 14, 12, 3, 5, 11, 10, 5, 14, 2, 7, 12 },
{ 7, 13, 13, 8, 14, 11, 3, 5, 0, 6, 6, 15, 9, 0, 10, 3,
  1, 4, 2, 7, 8, 2, 5, 12, 11, 1, 12, 10, 4, 14, 15, 9,
 10, 3, 6, 15, 9, 0, 0, 6, 12, 10, 11, 1, 7, 13, 13, 8,
 15, 9, 1, 4, 3, 5, 14, 11, 5, 12, 2, 7, 8, 2, 4, 14 },
{ 2, 14, 12, 11, 4, 2, 1, 12, 7, 4, 10, 7, 11, 13, 6, 1,
  8, 5, 5, 0, 3, 15, 15, 10, 13, 3, 0, 9, 14, 8, 9, 6,
  4, 11, 2, 8, 1, 12, 11, 7, 10, 1, 13, 14, 7, 2, 8, 13,
 15, 6, 9, 15, 12, 0, 5, 9, 6, 10, 3, 4, 0, 5, 14, 3 },
{ 12, 10, 1, 15, 10, 4, 15, 2, 9, 7, 2, 12, 6, 9, 8, 5,
  0, 6, 13, 1, 3, 13, 4, 14, 14, 0, 7, 11, 5, 3, 11, 8,
  9, 4, 14, 3, 15, 2, 5, 12, 2, 9, 8, 5, 12, 15, 3, 10,
  7, 11, 0, 14, 4, 1, 10, 7, 1, 6, 13, 0, 11, 8, 6, 13 },
{ 4, 13, 11, 0, 2, 11, 14, 7, 15, 4, 0, 9, 8, 1, 13, 10,
  3, 14, 12, 3, 9, 5, 7, 12, 5, 2, 10, 15, 6, 8, 1, 6,
  1, 6, 4, 11, 11, 13, 13, 8, 12, 1, 3, 4, 7, 10, 14, 7,
  10, 9, 15, 5, 6, 0, 8, 15, 0, 14, 5, 2, 9, 3, 2, 12 },
{ 13, 1, 2, 15, 8, 13, 4, 8, 6, 10, 15, 3, 11, 7, 1, 4,
 10, 12, 9, 5, 3, 6, 14, 11, 5, 0, 0, 14, 12, 9, 7, 2,
  7, 2, 11, 1, 4, 14, 1, 7, 9, 4, 12, 10, 14, 8, 2, 13,
  0, 15, 6, 12, 10, 9, 13, 0, 15, 3, 3, 5, 5, 6, 8, 11 }
};


static void xor( unsigned char *target, const unsigned char *src, int len )
{
    while(len--)
    {
        *target++ ^= *src++;
    }
}

static void permute( unsigned char target[],
        const unsigned char src[],
        const int permute_table[],
        int len )
{
    int i;
    for( i = 0; i<len*8; i++ )
    {
        if (GET_BIT(src, (permute_table[i] - 1 )))
        {
            SET_BIT(target, i);
        }
        else
        {
            CLEAR_BIT( target, i );
        }
    }
}

static void rol( unsigned char *target )
{
 int carry_left, carry_right;

 carry_left = ( target[ 0 ] & 0x80 ) >> 3;

 target[ 0 ] = ( target[ 0 ] << 1 ) | ( ( target[ 1 ] & 0x80 ) >> 7 );
 target[ 1 ] = ( target[ 1 ] << 1 ) | ( ( target[ 2 ] & 0x80 ) >> 7 );
 target[ 2 ] = ( target[ 2 ] << 1 ) | ( ( target[ 3 ] & 0x80 ) >> 7 );

 // special handling for byte 3
 carry_right = ( target[ 3 ] & 0x08 ) >> 3;
 target[ 3 ] = ( ( ( target[ 3 ] << 1 ) |
( ( target[ 4 ] & 0x80 ) >> 7 ) ) & ~0x10 ) | carry_left;

 target[ 4 ] = ( target[ 4 ] << 1 ) | ( ( target[ 5 ] & 0x80 ) >> 7 );
 target[ 5 ] = ( target[ 5 ] << 1 ) | ( ( target[ 6 ] & 0x80 ) >> 7 );
 target[ 6 ] = ( target[ 6 ] << 1 ) | carry_right;
}

#define DES_BLOCK_SIZE 8
#define DES_KEY_SIZE 8
#define EXPANSION_BLOCK_SIZE 6
#define PC1_KEY_SIZE 7
#define SUBKEY_SIZE 6

static void des_block_operate(const unsigned char plaintext[DES_BLOCK_SIZE],
                              unsigned char ciphertext[ DES_BLOCK_SIZE ],
                              const unsigned char keyp[DES_KEY_SIZE])
{
    unsigned char ip_block[DES_KEY_SIZE];
    unsigned char expansion_block [EXPANSION_BLOCK_SIZE];
    unsigned char substitution_block[DES_BLOCK_SIZE/2];
    unsigned char pbox_target[DES_BLOCK_SIZE/2];
    unsigned char recomb_box[DES_BLOCK_SIZE/2];

    unsigned char pc1key[PC1_KEY_SIZE];
    unsigned char subkey[SUBKEY_SIZE];
    int round;

    permute(pc1key, key, pc1_table, PC1_KEY_SIZE);
    for(round = 0; round < 16; round++){
        permute(expansion_block, ip_block+4, expansion_table, 6);
        rol(pc1key);
        if(!(round<=1 || round ==8 || round ==15))
        {
            rol(pc1key);
        }

        permute(subkey, pc1key, pc2_table, SUBKEY_SIZE);
        
        xor(expansion_block, subkey, 6);
        memset((void*)substitution_block, 0, DES_BLOCK_SIZE/2);


        substitution_block[0] = sbox[0][(expansion_block[0] & 0xFC)>>2] << 4;
substitution_block[ 0 ] =
     sbox[ 0 ][ ( expansion_block[ 0 ] & 0xFC ) >> 2 ] << 4;
   substitution_block[ 0 ] |=
     sbox[ 1 ][ ( expansion_block[ 0 ] & 0x03 ) << 4 |
     ( expansion_block[ 1 ] & 0xF0 ) >> 4 ];
   substitution_block[ 1 ] =
     sbox[ 2 ][ ( expansion_block[ 1 ] & 0x0F ) << 2 |
     ( expansion_block[ 2 ] & 0xC0 ) >> 6 ] << 4;
   substitution_block[ 1 ] |=
     sbox[ 3 ][ ( expansion_block[ 2 ] & 0x3F ) ];
   substitution_block[ 2 ] =
     sbox[ 4 ][ ( expansion_block[ 3 ] & 0xFC ) >> 2 ] << 4;
   substitution_block[ 2 ] |=
     sbox[ 5 ][ ( expansion_block[ 3 ] & 0x03 ) << 4 |
     ( expansion_block[ 4 ] & 0xF0 ) >> 4 ];
   substitution_block[ 3 ] =
     sbox[ 6 ][ ( expansion_block[ 4 ] & 0x0F ) << 2 |
     ( expansion_block[ 5 ] & 0xC0 ) >> 6 ] << 4;
   substitution_block[ 3 ] |=
     sbox[ 7 ][ ( expansion_block[ 5 ] & 0x3F ) ];

      permute(pbox_target, substitution_block, p_table, DES_BLOCK_SIZE/2);

      memcpy((void*)recomb_box, (void*)ip_block, DES_BLOCK_SIZE/2);
      memcpy( ( void * ) ip_block, ( void * ) ( ip_block + 4 ),
     DES_BLOCK_SIZE / 2 );
   xor( recomb_box, pbox_target, DES_BLOCK_SIZE / 2 );
   memcpy( ( void * ) ( ip_block + 4 ), ( void * ) recomb_box,
     DES_BLOCK_SIZE / 2 );

    }
  memcpy( ( void * ) recomb_box, ( void * ) ip_block, DES_BLOCK_SIZE / 2 );
 memcpy( ( void * ) ip_block, ( void * ) ( ip_block + 4 ), DES_BLOCK_SIZE / 2 );
 memcpy( ( void * ) ( ip_block + 4 ), ( void * ) recomb_box,
     DES_BLOCK_SIZE / 2 );

 // Final permutation (undo initial permutation)
 permute( ciphertext, ip_block, fp_table, DES_BLOCK_SIZE );

}

static void ror(unsigned char *target )
{
 int carry_left, carry_right;

 carry_right = ( target[ 6 ] & 0x01 ) << 3;

 target[ 6 ] = ( target[ 6 ] >> 1 ) | ( ( target[ 5 ] & 0x01 ) << 7 );
 target[ 5 ] = ( target[ 5 ] >> 1 ) | ( ( target[ 4 ] & 0x01 ) << 7 );
 target[ 4 ] = ( target[ 4 ] >> 1 ) | ( ( target[ 3 ] & 0x01 ) << 7 );

 carry_left = ( target[ 3 ] & 0x10 ) << 3;
 target[ 3 ] = ( ( ( target[ 3 ] >> 1 ) |
    ( ( target[ 2 ] & 0x01 ) << 7 ) ) & ~0x08 ) | carry_right;

 target[ 2 ] = ( target[ 2 ] >> 1 ) | ( ( target[ 1 ] & 0x01 ) << 7 );
 target[ 1 ] = ( target[ 1 ] >> 1 ) | ( ( target[ 0 ] & 0x01 ) << 7 );
 target[ 0 ] = ( target[ 0 ] >> 1 ) | carry_left;
}

typedef enum { OP_ENCRYPT, OP_DECRYPT } op_type;

static void des_operate( const unsigned char *input,
            int input_len,
            unsigned char *output,
            const unsigned char *key,
            op_type operation )
{
 unsigned char input_block[ DES_BLOCK_SIZE ];

 assert( !( input_len % DES_BLOCK_SIZE ) );

 while ( input_len )
 {
   memcpy( ( void * ) input_block, ( void * ) input, DES_BLOCK_SIZE );
   des_block_operate( input_block, output, key, operation );

   input += DES_BLOCK_SIZE;
   output += DES_BLOCK_SIZE;
   input_len -= DES_BLOCK_SIZE;
 }
}


 static void des_operate( const unsigned char *input,
            int input_len,
            unsigned char *output,
            const unsigned char *key,
            op_type operation )
{
 unsigned char input_block[ DES_BLOCK_SIZE ];

 assert( !( input_len % DES_BLOCK_SIZE ) );

 while ( input_len )
 {
   memcpy( ( void * ) input_block, ( void * ) input, DES_BLOCK_SIZE );
   des_block_operate( input_block, output, key, operation );

   input += DES_BLOCK_SIZE;
   output += DES_BLOCK_SIZE;
   input_len -= DES_BLOCK_SIZE;
 }
}   

void des_encrypt( const unsigned char *plaintext,
        const int plaintext_len,
        unsigned char *ciphertext,
        const unsigned char *key )
{
 unsigned char *padded_plaintext;
 int padding_len;

 // First, pad the input to a multiple of DES_BLOCK_SIZE

 padding_len = DES_BLOCK_SIZE - ( plaintext_len % DES_BLOCK_SIZE );
 padded_plaintext = malloc( plaintext_len + padding_len );

 // This implements NIST 800-3A padding
 memset( padded_plaintext, 0x0, plaintext_len + padding_len );
 padded_plaintext[ plaintext_len ] = 0x80;

 memcpy( padded_plaintext, plaintext, plaintext_len );

 des_operate( padded_plaintext, plaintext_len + padding_len, ciphertext,
              key, OP_ENCRYPT );
 free( padded_plaintext );
}

#ifdef TEST_DES
int main( int argc, char *argv[ ] )
{
 unsigned char *key;
 unsigned char *iv;
 unsigned char *input;
 unsigned char *output;
 int out_len, input_len;

 if ( argc < 4 )
 {
   fprintf( stderr, "Usage: %s <key> <iv> <input>\n", argv[ 0 ] );
   exit( 0 );
 }

 key = argv[ 1 ];
 iv = argv[ 2 ];
 input = argv[ 3 ];

 out_len = input_len = strlen( input );
 output = ( unsigned char * ) malloc( out_len + 1 );
 des_encrypt( input, input_len, output, iv, key );

 while ( out_len-- )
 {
   printf( "%.02x", *output++ );
 }
 printf( "\n" );

 return 0;
}
#endif






