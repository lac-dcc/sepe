/*
* The regex generator works by first assuming the entire string is always the
* same, and adapting that assumption as it reads more inputs. For example,
* imagine the following 3 lines of input:
*
*                                 10:45:AF
*                                 FF:35:AB
*                                 07:cb:09
*
* After reading the first line, our program assumes 10:45:AF is a fixed string.
* After reading the second line, our program updates that to a regex. Every byte
* that is different from the original becomes a [] class:
*
*                      [0-Z][0-Z]:[0-9][0-9]:[A-Z][A-Z]
*
* Upon reading the third line, we will update again to:
*
*                      [0-Z][0-Z]:[0-z][0-z]:[0-Z][0-Z]
*/

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

typedef struct Range {
	char start;
	char end;
	unsigned long count[256];
} Range;

enum Class {
	Class_Num = 0x1,
	Class_Lowercase = 0x2,
	Class_UpperCase = 0x4,
	Class_Punct = 0x8,
};

static int range_class(const Range range) {
	int class = 0;

	if (range.start >= '0' && range.start <= '9')
		class |= Class_Num;
	else if (range.start >= 'A' && range.start <= 'Z')
		class |= Class_UpperCase;
	else if (range.start >= 'a' && range.start <= 'z')
		class |= Class_Lowercase;
	else
		class |= Class_Punct;

	if (range.end >= '0' && range.end <= '9')
		class |= Class_Num;
	else if (range.end >= 'A' && range.end <= 'Z')
		class |= Class_UpperCase;
	else if (range.end >= 'a' && range.end <= 'z')
		class |= Class_Lowercase;
	else
		class |= Class_Punct;

	return class;
}

static void print_class(const Range range) {
	char start = 0;
	char end = 127;

	if (range.start >= '0' && range.start <= '9')
		start = '0';
	else if (range.start >= 'A' && range.start <= 'Z')
		start = 'A';
	else if (range.start >= 'a' && range.start <= 'z')
		start = 'a';
	else
		start = '!';

	if (range.end >= '0' && range.end <= '9')
		end = '9';
	else if (range.end >= 'A' && range.end <= 'Z')
		end = 'Z';
	else if (range.end >= 'a' && range.end <= 'z')
		end = 'z';
	else
		end = '}';

	printf("[%c-%c]", start, end);
}

static int is_special(const char ch) {
	return ch == '\\'
		|| ch == '['
		|| ch == '{'
		|| ch == ')'
		|| ch == '('
		|| ch == '+'
		|| ch == '*'
		|| ch == '?'
		|| ch == '.';
}

int main(int argc, const char* argv[]) {

	if (argc > 1) {
		printf("keybuilder\n");
		printf("\nDescription: keybuilder generates a regex from a series of strings separated by newlines\n");
		printf("\nExample usage: `./keybuilder < keys.txt`\n");
		printf("\nOptions:\n");
		printf("\n    -h    Print this help\n");
		printf("\n");
		if (!(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
			fprintf(stderr, "ERROR: unrecognized argument: %s\n", argv[1]);
			return 1;
		} else {
			return 0;
		}
	}

	char* line;
	size_t n;

	// begin by reading the first line
	ssize_t line_size = getline(&line, &n, stdin);
	if (line_size == -1) {
		if (errno != 0) {
			fprintf(stderr, "failed to read initial line from standard input: %s\n", strerror(errno));
		} else {
			fprintf(stderr, "must send strings in standard input!\n");
		}
		free(line);
		return 1;
	}

	Range* ranges = malloc(line_size * sizeof(*ranges));

	// set the initial ranges, according to the first line
	for (ssize_t i = 0; i < line_size - 1; ++i) {
		ranges[i].start = line[i];
		ranges[i].end = line[i];
	}


	// now, read every line, while updating the ranges. We assume all
	// lines have the same size, and exit with an error if they don't
	ssize_t in_bytes;
	while ((in_bytes = getline(&line, &n, stdin)) > -1) {
		if (in_bytes != line_size) {
			fprintf(stderr, "WARNING: lines have different size!\n");
		}
		line_size = line_size < in_bytes ? line_size : in_bytes;

		for (ssize_t i = 0; i < line_size - 1; ++i) {
			ranges[i].start = line[i] < ranges[i].start ? line[i] : ranges[i].start;
			ranges[i].end = line[i] > ranges[i].end ? line[i] : ranges[i].end;
			ranges[i].count[line[i]]++;
		}
	}
	free(line);

	// finally, group consecutive identical ranges together. For example,
	// `[0-9][0-9][0-9]` will turn into `[0-9]{3}`
	ssize_t i = 0;
	while (i < line_size - 1) {
		const Range range = ranges[i++];
		if (range.start == range.end) {
			if (is_special(range.start)) {
				putchar('\\');
			}
			putchar(range.start);
		} else {
			int repetitions = 1;
			Range other = ranges[i++];
			while (range_class(range) == range_class(other)) {
				other = ranges[i++];
				++repetitions;
			}
			--i;

			print_class(range);
			if (repetitions > 1) {
				printf("{%d}", repetitions);
			}
		}
	}
	puts("");

	for (ssize_t i = 0; i < line_size - 1; ++i) {
		ssize_t nonzeros = 0;
		for (int j = 0; j < 256; ++j)
			nonzeros += ranges[i].count[j] != 0;
		printf("%ld ", nonzeros);
	}
	puts("");

	free(ranges);
	return 0;
}
