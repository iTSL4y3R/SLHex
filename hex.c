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

// Macros for messages

#define PINFO "SLHex by SL 2020 (C)\n"
#define ADDRESS_WIDTH 8

#define PLOGO " ███████╗██╗     ██╗  ██╗███████╗██╗  ██╗\n" \
			  " ██╔════╝██║     ██║  ██║██╔════╝╚██╗██╔╝\n" \
			  " ███████╗██║     ███████║█████╗   ╚███╔╝ \n" \
			  " ╚════██║██║     ██╔══██║██╔══╝   ██╔██╗ \n" \
			  " ███████║███████╗██║  ██║███████╗██╔╝ ██╗\n" \
			  " ╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝"

#define HELPINFO "To redirect the input use > [file]\n" \
				 "width and read size should be numbers which are a power of two\n\n" \
				 "Argument list:\n" \
				 "\t-f [file] - specify the file\n" \
				 "\t-w [width] - specify the width\n" \
				 "\t-b [offset] - specify the reading offset\n" \
				 "\t-s [readsize] - specify the read size\n" \
				 "\t-h - hide the ANSI representation\n" \
				 "\t-l - hide SLHex (useless) logo."

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

// Forward declarations 
int  main(int argc, char** argv);
int  loadHex(char*);
void printTable(int width);

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
		else if (strcmp(argv[i], "-l") == 0) {
			++i;
			hidelogo = !hidelogo;
		}
		else {
			fprintf(stderr, ARG_ERR, argv[i]);
			return -1;
		}
	}
	
	puts(!hidelogo ? "\n" PLOGO "\n\n" PINFO : PINFO);
	if (!loadHex(bin_file)) {
		fputs(FOPEN_ERR, stderr);
		return -1;
	}
	printTable(width_out);

	printf("File summary: %s | size: %lu | read: %lu\n", bin_file, file_actual_size, read_bytes);

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
		exit(-1);
	}

	fseek(file, beg_out, SEEK_SET);

	hex = (char*)malloc(size_hex);
	if (!hex) {
		puts(MEM_ERR);
		exit(-1);
	}
	read_bytes = fread(hex, 1, size_hex, file);

	return 1;
}

void printTable(int width) {
	// A nested funtion, as supported by GCC
	void draw_line() {
		putc('\n', stdout);
		for (int col = 0; col < (2 + ADDRESS_WIDTH + 3 + (width * 3) - 1 + (show_char ? (3 + width) : 0)); ++col) {
			putc('-', stdout);
		}
		putc('\n', stdout);
	}
	
	uint64_t rowCount = floorl(size_hex / width);

	printf("Region: 0x%0" strmacro(ADDRESS_WIDTH) "lX - 0x%0" strmacro(ADDRESS_WIDTH) "lX (showing %lu bytes, omitting %lu bytes) -->", beg_out, beg_out + size_hex, size_hex, file_actual_size - size_hex);

	draw_line();
	printf("%-*s", ADDRESS_WIDTH + 2 + 3, "OFFSET:");

	for (int col = 0; col < width; ++col) {
		printf("%02X ", col);
	}

	if (show_char) {
		printf("%2c", ' ');
		for (int col = 0; col < width; ++col) {
			printf("%01X", col % 16);
		}
	}
	
	draw_line();

	for (uint64_t row = 0; row < rowCount; ++row) {
		printf("0x%0" strmacro(ADDRESS_WIDTH) "X | ", (unsigned)(beg_out + width * row));
		for (int col = 0; col < width; ++col) {
			printf("%02X ", (unsigned char)hex[width * row + col]);
		}
		if (show_char) {
			printf("| ");
			for (int col = 0; col < width; ++col) {
				char hex_char = hex[width * row + col];
				if (hex_char < 32 || hex_char == 0x7F)
					putc('.', stdout);
				else
					putc(hex_char, stdout);
			}
		}

		if (row != rowCount - 1) putc('\n', stdout);
	}

	draw_line();
}