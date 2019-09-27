#include <windows.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "security/id.h"

#include "util/hex.h"

static void print_usage(char** argv)
{
	fprintf(stderr,
		"Usage: %s <cmd> ...\n"
		"  gen: Generate a random pcbid\n"
		"  make <8 byte hex id>: Create a full pcbid (header + checksum) "
		"of the provided id\n",
		argv[0]);
}

int main(int argc, char** argv)
{
	struct security_id id;
	char* str;

	if (argc < 2) {
		print_usage(argv);
		return -1;
	}

	if (!strcmp(argv[1], "gen")) {
		srand(time(NULL));

		for (uint8_t i = 0; i < sizeof(id.id); i++) {
			id.id[i] = rand();
		}
	} else if (!strcmp(argv[1], "make")) {
		if (argc < 3) {
			print_usage(argv);
			return -2;
		}

		if (strlen(argv[2]) != 16) {
			fprintf(stderr, "Invaild length %d for id, must be 16\n",
				strlen(argv[2]));
			return -3;
		}

		hex_decode(id.id, sizeof(id.id), argv[2], strlen(argv[2]));
	} else {
		fprintf(stderr, "Invaild command '%s'\n", argv[1]);
		return -4;
	}

	security_id_prepare(&id);
	str = security_id_to_str(&id, false);
	printf("%s\n", str);
	free(str);
}