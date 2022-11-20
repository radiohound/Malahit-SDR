#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define DEFAULT_ADDR 0x8000000
#define BUFSZ 0x10000
uint8_t       buf[BUFSZ];

uint32_t      start_addr;
int           save_space;

void
addsum( char *s )
{
  uint8_t       h, l, sum = 0;

  s++;
  while( *s )
  {
    h = *s++ - '0';
    if( h > 9 )
      h -= 0x7;
    l = *s++ - '0';
    if( l > 9 )
      l -= 0x7;
    sum -= ( h << 4 ) | l;
  }
  sprintf( s, "%02X\n", sum );
}

int
non_ff_bytes( uint8_t * b, int len )
{
  int           l = len;

  while( l > 0 )
  {
    if( *( b + l - 1 ) != 0xff )
      break;
    l--;
  }
  if( len - l > 3 )
    return l;
  return len;
}

int
ff_buf( int len )
{
  int           i;

  for( i = 0; i < len; i++ )
    if( buf[i] != 0xff )
      return 0;
  return 1;
}

void
bin2hex( FILE * in, FILE * out )
{
  char          line[64];
  uint32_t      i, l, off, addr = start_addr;
  int           len;

  while( ( len = fread( buf, 1, BUFSZ, in ) ) > 0 )
  {
    if( save_space && ff_buf( len ) )
    {
      addr += len;
      continue;
    }
    sprintf( line, ":02000004%04X", addr / BUFSZ );
    addsum( line );
    fputs( line, out );
    for( off = 0; off < len; off += 0x10 )
    {
      l = ( len >= off + 0x10 ) ? 0x10 : len - off;
      if( save_space )
        l = non_ff_bytes( &buf[off], l );
      if( !l )
	continue;
      sprintf( line, ":%02X%04X00", l, off );
      for( i = 0; i < l; i++ )
	sprintf( line + 9 + 2 * i, "%02X", buf[off + i] );
      addsum( line );
      fputs( line, out );
    }
    addr += len;
  }
  strcpy( line, ":00000001" );
  addsum( line );
  fputs( line, out );
}

void
print_help( char *progname )
{
  printf( "Binary STM32 to iHex\n" );
  printf( "Options:\n" );
  printf( "\t-a #\tloading address (default is 0x%x)\n", DEFAULT_ADDR );
  printf( "\t-s\tsave space by ommiting regions of 0xff\n" );
  printf( "\t-h\thelp\n" );
  printf( "Example: %s binary_image\n", progname );
}

int
main( int argc, char **argv )
{
  FILE         *in, *out;
  char         *p, fout[128];
  int           option;

  start_addr = DEFAULT_ADDR;
  do
  {
    option = getopt( argc, argv, "hsa:" );
    switch ( option )
    {
      case 'a':		// start address
	start_addr = strtol( optarg, NULL, 16 );
	break;
      case 's':
	save_space++;
	break;
      case 'h':
	print_help( argv[0] );
	return 0;
      case EOF:		// no more options
	break;
      default:
	fprintf( stderr, "getopt returned impossible value: %d ('%c')\n",
	    option, option );
    }
  }
  while( option != EOF );

  if( optind == argc )
  {
    fprintf( stderr, "No input file specifed.\n\n" );
    print_help( argv[0] );
    return -1;
  }

  if( ( in = fopen( argv[optind], "rb" ) ) == NULL )
  {
    fprintf( stderr, "Cannot open file '%s'\n", argv[optind] );
    return -2;
  }
  strcpy( fout, argv[optind] );
  if( ( p = strrchr( fout, '.' ) ) == NULL )
    p = fout + strlen( fout );
  strcpy( p, ".hex" );
  if( ( out = fopen( fout, "wt" ) ) == NULL )
  {
    fprintf( stderr, "Cannot create file '%s'\n", fout );
    return -3;
  }
  bin2hex( in, out );
  fclose( out );
  fclose( in );
  exit( 0 );
}
