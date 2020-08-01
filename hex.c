/*

Copyright 2020 © Sergey Lafin

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include <math.h>

#include <stdint.h>
#include <stdarg.h>

// Macros for messages

#define PINFO "SLHex by SL 2020 (C)\n"
#define ADDRESS_WIDTH 8

#define PLOGO " ███████╗██╗     ██╗  ██╗███████╗██╗  ██╗\n" \
			  " ██╔════╝██║     ██║  ██║██╔════╝╚██╗██╔╝\n" \
			  " ███████╗██║     ███████║█████╗   ╚███╔╝ \n" \
			  " ╚════██║██║     ██╔══██║██╔══╝   ██╔██╗ \n" \
			  " ███████║███████╗██║  ██║███████╗██╔╝ ██╗\n" \
			  " ╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝"

#define HELPINFO "Argument list:\n" \
				 "\t-f [file] - specify the file\n" \
				 "\t-w [width] - specify the width\n" \
				 "\t-b [offset] - specify the reading offset\n" \
				 "\t-s [readsize] - specify the read size\n" \
				 "\t-o [file]- output both to stdout and a file\n" \
				 "\t-h - hide the ANSI representation\n" \
				 "\t-l - hide SLHex (useless) logo"

#define FOPEN_ERR "An error occured while reading the file.\n"
#define ARG_ERR "Undefined argument %s\n"
#define MEM_ERR "Memory allocation error"

#define strmacroname(x) #x
#define strmacro(x) strmacroname(x)

#define streq(x, y) (strcmp(x, y) == 0)

static char* hex; // Char array to store bytes from the file
size_t size_hex;  // Number of bytes to be read

short width_out = 16; // width of the output table
uint8_t show_char = 1; // Show ASCII representation?
char* bin_file; // Name of the file to be read

size_t beg_out = 0; // File seek

size_t file_actual_size = 0; // The actual file size
size_t read_bytes = 0; // How many bytes were actually read from the file

uint8_t hidelogo = 0; // Hide the logo?

FILE* output_stream = NULL; // where the output goes

// Forward declarations 
int  main(int argc, char** argv);
int  loadHex(char*);
void printTable(int width);

//Functions to both output to stdout and a given file.

static inline void stdputc(int ch, FILE* file) {
	if (file) putc(ch, file);
	putchar(ch);
}

static inline void stdputs(const char* string, FILE* file) {
	if (file) fputs(string, file);
	puts(string);
}

static inline void stdprintf(FILE* file, const char* fmt, ...) {
	va_list arg;
	
	if (file) {
		va_start(arg, fmt);
		vfprintf(file, fmt, arg);
		va_end(arg);
	}

	va_start(arg, fmt);
	vprintf(fmt, arg);
	va_end(arg);
}

int main(int argc, char** argv) {
	if (argc < 2) {
		return -1;
	}
	for (uint16_t i = 1; i < argc; ++i) {
		if (streq(argv[i], "--help")) {
			puts(PINFO HELPINFO);
			return 0;
		}
		else if (streq(argv[i], "-w")) {
			++i;
			width_out = atoi(argv[i]);
		}
		else if (streq(argv[i], "-f")) {
			++i;
			bin_file = malloc(strlen(argv[i]));
			strcpy(bin_file, argv[i]);
		}
		else if (streq(argv[i], "-s")) {
			++i;
			size_hex = atol(argv[i]);
		}
		else if (streq(argv[i], "-b")) {
			++i;
			sscanf(argv[i], "%li", &beg_out);
		}
		else if (streq(argv[i], "-h")) {
			++i;
			show_char = !show_char;
		}
		else if (streq(argv[i], "-l")) {
			++i;
			hidelogo = !hidelogo;
		}
		else if (streq(argv[i], "-o"))  {
			++i;
			output_stream = fopen(argv[i], "w+");
			if (!output_stream) {
				fputs(FOPEN_ERR, stderr);
				return -1;
			}
		}
		else {
			fprintf(stderr, ARG_ERR, argv[i]);
			return -1;
		}
	}
	
	stdputs(!hidelogo ? "\n" PLOGO "\n\n" PINFO : PINFO, output_stream);
	if (!loadHex(bin_file)) {
		fputs(FOPEN_ERR, stderr);
		if (output_stream) fclose(output_stream);
		return -1;
	}
	printTable(width_out);
	free(hex);

	stdprintf(output_stream, "File summary: %s | size: %lu | read: %lu\n", bin_file, file_actual_size, read_bytes);
	if (output_stream) fclose(output_stream);

	return 0;
}

int loadHex(char* path) {
	FILE* file = fopen(path, "rb");
	if (!file) {
		return 0;
	}

	// Count file size
	fseek(file, 0, SEEK_END);
	file_actual_size = ftell(file);
	if (size_hex == 0) size_hex = file_actual_size;
	rewind(file);

	if (beg_out > file_actual_size) {
		fclose(file);
		return 0;
	}

	fseek(file, beg_out, SEEK_SET);

	hex = (char*)malloc(size_hex);
	if (!hex) {
		puts(MEM_ERR);
		fclose(file);
		return 0;
	}
	read_bytes = fread(hex, 1, size_hex, file);
	fclose(file);

	return 1;
}

void printTable(int width) {
	// A nested funtion, as supported by GCC
	void draw_line() {
		stdputc('\n', output_stream);
		for (int col = 0; col < (2 + ADDRESS_WIDTH + 3 + (width * 3) - 1 + (show_char ? (3 + width) : 0)); ++col) {
			stdputc('-', output_stream);
		}
		stdputc('\n', output_stream);
	}
	
	uint64_t rowCount = ceil((float)size_hex / (float)width);

	stdprintf(output_stream, "Region: 0x%0" strmacro(ADDRESS_WIDTH) "lX - 0x%0" strmacro(ADDRESS_WIDTH) "lX (showing %lu bytes, omitting %lu bytes) -->", beg_out, beg_out + size_hex, size_hex, file_actual_size - size_hex);

	draw_line();
	stdprintf(output_stream, "%-*s", ADDRESS_WIDTH + 2 + 3, "OFFSET:");

	for (int col = 0; col < width; ++col) {
		stdprintf(output_stream, "%02X ", col);
	}

	if (show_char) {
		stdprintf(output_stream, "%2c", ' ');
		for (int col = 0; col < width; ++col) {
			stdprintf(output_stream, "%01X", col % 16);
		}
	}
	
	draw_line();

	for (uint64_t row = 0; row < rowCount; ++row) {
		stdprintf(output_stream, "0x%0" strmacro(ADDRESS_WIDTH) "X | ", (unsigned)(beg_out + width * row));
		for (int col = 0; col < width; ++col) {
			size_t elem = width * row + col;
			if (elem < size_hex)
				stdprintf(output_stream, "%02X ", (unsigned char)hex[elem]);
			else
				stdprintf(output_stream, "%3c", ' ');
		}
		if (show_char) {
			stdprintf(output_stream, "| ");
			for (int col = 0; col < width; ++col) {
				size_t elem = width * row + col;
				char hex_char = hex[elem];
				
				if (elem < size_hex) {
					if (hex_char < 32 || hex_char == 0x7F)
						stdputc('.', output_stream);
					else
						stdputc(hex_char, output_stream);
				}
			}
		}

		if (row != rowCount - 1) stdputc('\n', output_stream);
	}

	draw_line();
}