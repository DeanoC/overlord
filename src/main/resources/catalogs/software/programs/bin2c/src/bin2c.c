#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

static void Abort (char *fmt,...)
{
  va_list args;
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  exit (1);
}

int main (int argc, char **argv) {
	FILE  *inFile;
	FILE  *outFile = stdout;
	time_t now     = time (NULL);
	int    ch;
	const char searchchar0 = '/';
	const char searchchar1 = '\\';
	const char searchchar2 = '.';
	const char replacechar = '_';
	char *ptr;    // current position in string

	if (argc != 4)
		Abort ("Usage: %s bin-file result name", argv[0]);

	if ((inFile = fopen(argv[1],"rb")) == NULL)
		Abort ("Cannot open %s\n", argv[1]);

	if ((outFile = fopen(argv[2],"wt")) == NULL)
		Abort ("Cannot open %s\n", argv[2]);

	fprintf (outFile,
			"// data statements for file %s at %.24s\n"
			"// Generated by BIN2C, G.Vanem 1995 modified by Deano Calver 2011-2022\n"
			"#include <stdint.h>\n",
			argv[1], ctime(&now));
    ptr = argv[3];
    // Loop until end of string replacing \/. to _
    while (*ptr != '\0') {
		if(*ptr == searchchar0 || *ptr == searchchar1 || *ptr == searchchar2 )
             *ptr = replacechar;
        ptr++;
    }
	// remove extension part of name
    while (*ptr != '_') {
        --ptr;
    }
	if( *ptr == '_' ) {
		*ptr = '\0';
	}

	fprintf( outFile, "static uint8_t const binary_data_%s_data[] = {", argv[3] );
	unsigned int i = 0;

	while ((ch = fgetc(inFile)) != EOF) {
		if (i++ % 12 == 0) {
			fputs ("\n  ", outFile);
		}
		fprintf (outFile, "0x%02X,", ch);
	}

	fputc ('\n', outFile);
	fprintf( outFile, "};\n" );

	fprintf( outFile, "uint8_t const * binary_data_%s = binary_data_%s_data;\n", argv[3], argv[3] );
	fprintf( outFile, "unsigned int const binary_data_%s_sizeof = %i;\n", argv[3], i );

	fclose (inFile);
	return (0);
}