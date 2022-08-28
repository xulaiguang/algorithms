#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#define R 256

typedef int (*char_at_func_t)(const unsigned char *, int, int, int);

static int sa_char_at(const unsigned char *str, int len, int suffix, int i)
{
	assert(suffix < len && i < len && suffix + i >= 0);
	return suffix + i >= len ? -1 : str[suffix+i];
}

static int csa_char_at(const unsigned char *str, int len, int suffix, int i)
{
	assert(suffix < len && i < len && suffix + i >= 0);
	return str[(suffix + i) % len];
}

// circular suffix array
// using Radix sort + binary lifting
// O(nlgn)
int *csa_sort(const unsigned char *str, int len)
{
	int count_len = len > R ? len + 1 : R + 1;

	int *csa = malloc(len * sizeof(int));
	int *x = malloc(len * sizeof(int));
	int *y = malloc(len * sizeof(int));
	int *count = malloc(count_len * sizeof(int));
	memset(count, 0, count_len * sizeof(int));
	
	for (int i = 0; i < len; i++) {
		x[i] = str[i];
		count[str[i] + 1]++;
	}

	for (int i = 0; i < R; i++) {
		count[i+1] += count[i];
	}

	for (int i = 0; i < len; i++) {
		csa[count[str[i]]] = i;
		count[str[i]]++;
	}

	int r = R;
	for (int k = 1; k < len; k *= 2) {
		// each loop is an LSD sort with d = 2
		//
		// LSD sort d = 0
		for (int i = 0; i < len; i++) {
			y[i] = (csa[i] + len - k) % len;
		}

		memset(count, 0, (r+1)*sizeof(int));

		// LSD sort d = 1
		for (int i = 0; i < len; i++) {
			count[x[y[i]] + 1]++;
		}

		for (int i = 0; i < r; i++) {
			count[i+1] += count[i];
		}

		for (int i = 0; i < len; i++) {
			csa[count[x[y[i]]]] = y[i];
			count[x[y[i]]]++;
		}

		// swap(x, y)
		memcpy(y, x, len * sizeof(int));

		// construct new x[]
		r = 0;
		x[csa[0]] = 0;
		for (int i = 1; i < len; i++) {
			if (!(y[csa[i-1]] == y[csa[i]] && y[(csa[i-1]+k) % len] == y[(csa[i]+k) % len]))
				r++;
			x[csa[i]] = r;
		}

		assert(r < len);

		if (r == len - 1) {
			break;
		}
	}

	free(count);
	free(x);
	free(y);

	return csa;
}

// use LSD-sort to sort circular suffix array
// O(n^2)
int *lsd_sort(const unsigned char *str, int len)
{
	int count[R+1] = { 0 };
	int *sa = malloc(len * sizeof(int));
	int *aux = malloc(len * sizeof(int));

	for (int i = 0; i < len; i++)
		sa[i] = i;

	for (int d = len - 1; d >= 0; d--) {
		memset(count, 0, (R+1) * sizeof(int));

		for (int i = 0; i < len; i++) {
			int rdx = csa_char_at(str, len, sa[i], d);
			count[rdx+1]++;
		}

		for (int i = 0; i < R; i++) {
			count[i+1] += count[i];
		}

		for (int i = 0; i < len; i++) {
			int rdx = csa_char_at(str, len, sa[i], d);
			aux[count[rdx]] = sa[i];
			count[rdx]++;
		}

		memcpy(sa, aux, len * sizeof(int));
	}

	free(aux);

	return sa;
}

static void exch(int *a, int x, int y)
{
	int tmp = a[x];
	a[x] = a[y];
	a[y] = tmp;
}

// [lo, hi)
static void sort(const unsigned char *str, int len, int *sa, int lo, int hi, int d, char_at_func_t char_at)
{
	if (d >= len)
		return;
	if (hi <= lo + 1)
		return;

	int lt = lo, gt = hi - 1;
	int v = char_at(str, len, sa[lo], d);
	int i = lo + 1;

	while (i <= gt) {
		int t = char_at(str, len, sa[i], d);
		if (t < v)
			exch(sa, lt++, i++);
		else if (t > v) {
			exch(sa, i, gt--);
		} else {
			i++;
		}
	}

	sort(str, len, sa, lo, lt, d, char_at);
	if (v >= 0)
		sort(str, len, sa, lt, gt+1, d+1, char_at);
	sort(str, len, sa, gt+1, hi, d, char_at);
}

// sorting the suffix array or the circular suffix array
// depends on char_at()
int *__quick3way(const unsigned char *str, int len, char_at_func_t char_at)
{
	int *sa = (int *)malloc(len * sizeof(int));
	assert(sa);

	for (int i = 0; i < len; i++) {
		sa[i] = i;
	}

	sort(str, len, sa, 0, len, 0, char_at);

	return sa;
}

int *csa_quick3way(const unsigned char *str, int len)
{
	return __quick3way(str, len, csa_char_at);
}

int *sa_quick3way(const unsigned char *str, int len)
{
	return __quick3way(str, len, sa_char_at);
}

static int equal(const int *y, int len, int a, int b, int k)
{
	if (a + k < len && b + k >= len) {
		return 0;
	}

	if (a + k >= len && b + k < len) {
		return 0;
	}

	if (a + k >= len && b + k >= len) {
		return y[a] == y[b];
	}

	return y[a] == y[b] && y[a + k] == y[b + k];
}

// suffix array
// radix sort + binary lifting
// O(nlgn)
int *sa_sort(const unsigned char *str, int len)
{
	int count_len = len > R ? len + 1 : R + 1;

	int *sa = malloc(len * sizeof(int));
	int *x = malloc(len * sizeof(int));
	int *y = malloc(len * sizeof(int));
	int *count = malloc(count_len * sizeof(int));
	memset(count, 0, count_len * sizeof(int));
	
	for (int i = 0; i < len; i++) {
		x[i] = str[i];
		count[str[i] + 1]++;
	}

	for (int i = 0; i < R; i++) {
		count[i+1] += count[i];
	}

	for (int i = 0; i < len; i++) {
		sa[count[str[i]]] = i;
		count[str[i]]++;
	}

	int r = R;
	for (int k = 1; k < len; k *= 2) {
		// each loop is an LSD sort with d = 2
		//
		// LSD sort d = 0
		int p = 0;
		for (int i = len - k; i < len; i++) {
			y[p] = i;
			p++;
		}

		for (int i = 0; i < len; i++) {
			if (sa[i] >= k) {
				y[p] = sa[i] - k;
				p++;
			}
		}

		memset(count, 0, (r+1)*sizeof(int));

		// LSD sort d = 1
		for (int i = 0; i < len; i++) {
			count[x[y[i]] + 1]++;
		}

		for (int i = 0; i < r; i++) {
			count[i+1] += count[i];
		}

		for (int i = 0; i < len; i++) {
			sa[count[x[y[i]]]] = y[i];
			count[x[y[i]]]++;
		}

		// construct new x[]
		memcpy(y, x, len * sizeof(int));

		x[sa[0]] = r = 0;
		for (int i = 1; i < len; i++) {
			if (!equal(y, len, sa[i-1], sa[i], k)) {
				r++;
			}
			x[sa[i]] = r;
		}

		assert(r < len);

		if (r == len - 1) {
			break;
		}
	}

	free(count);
	free(x);
	free(y);

	return sa;
}

static char random_char()
{
	static int init = 0;
	if (init == 0) {
		init = 1;
		srand(time(NULL));
	}

	return rand() % 256;
}

static unsigned char *generate_random_str(int len)
{
	char *str = malloc(len);
	for (int i = 0; i < len; i++) {
		str[i] = random_char();
	}
	return str;
}

int main(int argc, char *argv[])
{
	unsigned char *str = "ABRACADABRA!";
	int len = 12;
	if (argc == 2) {
		len = atoi(argv[1]);
		str = generate_random_str(len);
	}
	int *sa = csa_quick3way(str, len);

	for (int i = 0; i < strlen(str); i++) {
		printf("sa[%d]: %d\n", i, sa[i]);
	}

	free(sa);

	return 0;
}
