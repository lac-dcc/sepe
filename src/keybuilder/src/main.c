#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

typedef struct Range {
	char start;
	char end;
} Range;

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
			return 1;
		}

		for (ssize_t i = 0; i < line_size - 1; ++i) {
			ranges[i].start = line[i] < ranges[i].start ? line[i] : ranges[i].start;
			ranges[i].end = line[i] > ranges[i].end ? line[i] : ranges[i].end;
		}
	}
	free(line);

	for (ssize_t i = 0; i < line_size - 1; ++i) {
		Range range = ranges[i];
		if (range.start == range.end) {
			printf("%c", range.start);
		} else {
			printf("[%c-%c]", range.start, range.end);
		}
	}
	puts("");

	free(ranges);
	return 0;
}
