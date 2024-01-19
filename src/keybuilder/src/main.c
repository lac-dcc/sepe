#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

typedef struct Range {
	char start;
	char end;
} Range;

enum Class {
	Class_Num = 0x1,
	Class_Lowercase = 0x2,
	Class_UpperCase = 0x4,
	Class_Punct = 0x8,
};

int range_class(const Range range) {
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

void print_class(const Range range) {
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

int is_special(const char ch) {
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

int main(void) {

	char* line;
	size_t n;

	const ssize_t line_size = getline(&line, &n, stdin);
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

	for (ssize_t i = 0; i < line_size - 1; ++i) {
		ranges[i].start = line[i];
		ranges[i].end = line[i];
	}


	ssize_t in_bytes;
	while ((in_bytes = getline(&line, &n, stdin)) > -1) {
		if (in_bytes != line_size) {
			fprintf(stderr, "ERROR: all lines must have the exact same size!\n");
			free(line);
			free(ranges);
			return 1;
		}

		for (ssize_t i = 0; i < line_size - 1; ++i) {
			ranges[i].start = line[i] < ranges[i].start ? line[i] : ranges[i].start;
			ranges[i].end = line[i] > ranges[i].end ? line[i] : ranges[i].end;
		}
	}
	free(line);

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

	free(ranges);
	return 0;
}
