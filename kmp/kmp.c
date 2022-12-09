/*
 * {{{ Copyright (C) 2022 AliOS Project. All rights reserved. }}}
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static void print_next(const char *pattern, unsigned int *next)
{
	int j = 0;
	for (j = 0; j < strlen(pattern); j++) {
		printf("%c ", pattern[j]);
	}
	printf("\n");
	for (j = 0; j < strlen(pattern); j++) {
		printf("%u ", next[j]);
	}
	printf("\n");
}

unsigned int *build_next(const char *pattern)
{
	unsigned int pattern_len = strlen(pattern);
	unsigned int i = 1;
	unsigned int prefix = 0;

	unsigned int *next = (unsigned int *)malloc(pattern_len * sizeof(unsigned int));
	if (!next)
		return NULL;

	next[0] = 0;
	i = 1;
	prefix = 0;

	while (i < pattern_len) {
		if (pattern[i] == pattern[prefix]) {
			prefix++;
			next[i] = prefix;
			i++;
		} else {
			if (prefix == 0) {
				next[i] = 0;
				i++;
			} else {
				prefix = next[prefix-1];
			}
		}
	}

	return next;
}

int kmp(const char *str, const char *pattern)
{
	unsigned int i = 0, j = 0;
	unsigned int str_len = strlen(str);
	unsigned int pattern_len = strlen(pattern);
	unsigned int *next = build_next(pattern);

	if (!next)
		return -ENOMEM;

	print_next(pattern, next);

	while (i < str_len) {
		if (str[i] == pattern[j]) {
			i++;
			j++;

			if (j == pattern_len) {
				free(next);
				return i - j;
			}
		} else {
			if (j == 0) {
				i++;
			} else {
				j = next[j-1];
			}
		}
	}

	free(next);
	return -ESRCH;
}

int main(int argc, char *argv[])
{
	const char *str = argv[2];
	const char *pattern = argv[1];
	int ret = kmp(str, pattern);
	printf("ret: %d\n", ret);
	if (ret >= 0) {
		puts(&str[ret]);
	}

	return 0;
}
