#include <string.h>
#include <stdio.h>
#include <assert.h>

void lcstring(const char *s1, const char *s2)
{
	int s1len = strlen(s1);
	int s2len = strlen(s2);

	int dp[64][64];
	int max = 0, maxi = 0, maxj = 0;
	memset(dp, 0, sizeof(dp));

	for (int i = 0; i < s1len; i++) {
		for (int j = 0; j < s2len; j++) {
			if (s1[i] == s2[j]) {
				dp[i+1][j+1] = dp[i][j]+1;
			} else {
				dp[i+1][j+1] = 0;
			}

			if (max < dp[i+1][j+1]) {
				max = dp[i+1][j+1];
				maxi = i;
				maxj = j;
			}
		}
	}

	printf("max: %d: %d, %d(", max, maxi-max+1, maxj-max+1);
	for (int i = maxi - max+1; i <= maxi; i++) {
		putchar(s1[i]);
	}
	printf(")\n");
}

static int max(int a, int b)
{
	return a > b ? a : b;
}

void lcsequence(const char *s1, const char *s2)
{
	int s1len = strlen(s1);
	int s2len = strlen(s2);

	char seq[64] = { 0 };
	int dp[64][64];
	memset(dp, 0, sizeof(dp));

	for (int i = 0; i < s1len; i++) {
		for (int j = 0; j < s2len; j++) {
			if (s1[i] == s2[j]) {
				dp[i+1][j+1] = dp[i][j] + 1;
			} else {
				dp[i+1][j+1] = max(dp[i][j+1], dp[i+1][j]);
			}
		}
	}

	int maxlen = dp[s1len][s2len];
	printf("maxlen: %d\n", maxlen);

	int i = s1len - 1;
	int j = s2len - 1;

	while (i >= 0 && j >= 0 && maxlen > 0) {
		if (dp[i+1][j+1] == 0)
			break;

		if (s1[i] == s2[j]) {
			assert(dp[i+1][j+1] == dp[i][j] + 1);
			seq[maxlen - 1] = s1[i];
			i--;
			j--;
			maxlen--;
		} else {
			if (dp[i][j+1] == dp[i+1][j+1]) {
				i--;
			} else {
				j--;
			}
		}
	}
	printf("lcsequence: %s\n", seq);
}

int main(int argc, char *argv[])
{
	const char *s1 = "AGCAT";
	const char *s2 = "GAC";
	if (argc == 3) {
		s1 = argv[1];
		s2 = argv[2];
	}

	lcsequence(s1, s2);

	return 0;
}
