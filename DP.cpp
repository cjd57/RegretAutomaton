#include<iostream>
#include<vector>
#include<unordered_map>
#include<array>
#include<algorithm>
#include<numeric>
#include<set>
#include<functional>
#include<queue>
using namespace std;
typedef long long ll;
const ll NEG = -4e18;
ll dp[300][300][300], ndp[300][300][300];

int main() {
	ios::sync_with_stdio(false);
	cin.tie(nullptr);
	int n, K, L;
	cin >> n >> K >> L;
	if (n > 300)
		return -1;
	vector<ll> a(n + 1), b(n + 1);
	for (int i = 1; i <= n; i++)
		cin >> a[i];
	for (int i = 1; i <= n; i++)
		cin >> b[i];


	for (int j = 0; j <= K; j++)
		for (int k = 0; k <= K; k++)
			for (int l = 0; l <= K; l++)
				dp[j][k][l] = NEG;
	dp[0][0][0] = 0;

	for (int i = 1; i <= n; i++) {
		for (int j = 0; j <= K; j++)
			for (int k = 0; k <= K; k++)
				for (int l = 0; l <= K; l++)
					ndp[j][k][l] = NEG;

		for (int j = 0; j <= K; j++) {
			for (int k = 0; k <= K; k++) {
				for (int l = 0; l <= K; l++) {
					ll cur = dp[j][k][l];
					if (cur == NEG)
						continue;

					ndp[j][k][l] = max(ndp[j][k][l], cur);

					if (j + 1 <= K)
						ndp[j + 1][k][l] = max(ndp[j + 1][k][l], cur + a[i]);

					if (k + 1 <= K)
						ndp[j][k + 1][l] = max(ndp[j][k + 1][l], cur + b[i]);

					if (j + 1 <= K && k + 1 <= K && l + 1 <= K)
						ndp[j + 1][k + 1][l + 1] = max(ndp[j + 1][k + 1][l + 1], cur + a[i] + b[i]);
				}
			}
		}

		for (int j = 0; j <= K; j++)
			for (int k = 0; k <= K; k++)
				for (int l = 0; l <= K; l++)
					dp[j][k][l] = ndp[j][k][l];
	}
	ll ans = NEG;
	for (int l = L; l <= K; l++)
		ans = max(ans, dp[K][K][l]);
	cout << ans << '\n';
	return 0;
}