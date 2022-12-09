/*
 * {{{ Copyright (C) 2022 AliOS Project. All rights reserved. }}}
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

struct nqueens {
	uint32_t n;
	uint32_t total;
	uint32_t *column;
	uint32_t *diagnoal;
	uint32_t *antidiagnoal;
};

int nqueens_init(struct nqueens *nq, uint32_t n)
{
	nq->n = n;
	nq->total = 0;
	nq->column = malloc(sizeof(uint32_t) * n);
	nq->diagnoal = malloc(sizeof(uint32_t) * n);
	nq->antidiagnoal = malloc(sizeof(uint32_t) * n);
	memset(nq->column, 0, sizeof(uint32_t) * n);
	memset(nq->diagnoal, 0, sizeof(uint32_t) * n);
	memset(nq->antidiagnoal, 0, sizeof(uint32_t) * n);

	return 0;
}

static void __nqueens_search(struct nqueens *nq, uint32_t row)
{
	uint32_t avail = nq->column[row] | nq->diagnoal[row]
		| nq->antidiagnoal[row];
	avail = ~avail;

	while (avail) {
		uint32_t i = __builtin_ctz(avail); // count trailing zeros
		if (i >= nq->n)
			break;

		if (row == nq->n - 1) {
			nq->total++;
		} else {
			uint32_t mask = 1 << i;
			nq->column[row+1] = nq->column[row] | mask;
			nq->diagnoal[row+1] = (nq->diagnoal[row] | mask) >> 1;
			nq->antidiagnoal[row+1] = (nq->antidiagnoal[row] | mask) << 1;
			__nqueens_search(nq, row+1);
		}

		avail &= (avail - 1);
	}
}

void nqueens_search(struct nqueens *nq)
{
	__nqueens_search(nq, 0);
}

int nqueens_exit(struct nqueens *nq)
{
	free(nq->column);
	free(nq->diagnoal);
	free(nq->antidiagnoal);

	memset(nq, 0, sizeof(*nq));
	return 0;
}

int main()
{
	struct nqueens nq;
	nqueens_init(&nq, 8);
	nqueens_search(&nq);

	printf("total: %u\n", nq.total);
	nqueens_exit(&nq);
	return 0;
}
