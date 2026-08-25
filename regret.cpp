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
#define int long long
typedef priority_queue<array<int, 2> > heap;
heap pq[6];
const int maxn = 4e6 + 10;
int n, K, L, sum, a[maxn], b[maxn], st[maxn][2];
array<int, 2>tmpa[maxn], tmpb[maxn];

//trans[i]表示pq[i]这个堆维护的具体式子
function<void(heap &, int)> trans[] = {
	[](heap & h, int id) { h.push({b[id], id}); },
	[](heap & h, int id) { h.push({-b[id], id}); },
	[](heap & h, int id) { h.push({a[id], id}); },
	[](heap & h, int id) { h.push({-a[id], id}); },
	[](heap & h, int id) { h.push({a[id] + b[id], id}); },
	[](heap & h, int id) {
		h.push({-a[id] - b[id], id});
	}
};

//判断元素id是否符合pq[i]的要求
function<bool(int)> ok[] = {
	[](int id) { return st[id][0] == 1 && st[id][1] == 0; },  // 堆0: da
	[](int id) { return st[id][0] == 0 && st[id][1] == 1; },  // 堆1: db
	[](int id) { return st[id][0] == 0 && st[id][1] == 1; },  // 堆2: db
	[](int id) { return st[id][0] == 1 && st[id][1] == 0; },  // 堆3: da
	[](int id) { return st[id][0] == 0 && st[id][1] == 0; },  // 堆4: na&nb
	[](int id) {
		return st[id][0] == 2 && st[id][1] == 2;    // 堆5: sa&sb
	}
};

//to[i]能够把输入进来的状态转化为对应堆的输出状态,0表示未选，1是选了其他，2是选了下标相同的
function<void(int)> to[] = {
	[](int id) { st[id][0] = st[id][1] = 2; },
	[](int id) { st[id][1] = 0; },
	[](int id) { st[id][0] = st[id][1] = 2; },
	[](int id) { st[id][0] = 0; },
	[](int id) { st[id][0] = st[id][1] = 2; },
	[](int id) {
		st[id][0] = st[id][1] = 0;
	}
};

//sm[i]表示第i种操作需要组合的pq
vector<int>sm[4] = { {0, 1}, {2, 3}, {1, 3, 4}, {0, 2, 5} };
int nm, op, ne[6];
void work(int id) {
	int s = 0;
	for (int i = 0; i < sm[id].size(); ++i) {
		if (pq[sm[id][i]].empty())
			return;
		s += pq[sm[id][i]].top()[0];
	}
	if (s > nm) {
		nm = s;
		op = id;
		memset(ne, 0, sizeof ne);
		for (int i = 0; i < sm[id].size(); ++i)
			ne[sm[id][i]] = pq[sm[id][i]].top()[1];
	}
}
void upd() {
	sum += nm;
	for (int i = 0; i < sm[op].size(); ++i) {
		int id = ne[sm[op][i]];
		to[sm[op][i]](id);
		pq[sm[op][i]].pop();
		for (int j = 0; j < 6; ++j)
			if (ok[j](id))
				trans[j](pq[j], id);
	}
}
void clear() {
	n = K = L = sum = 0;
	memset(a, 0, sizeof a);
	memset(b, 0, sizeof b);
	memset(st, 0, sizeof st);
	memset(tmpa, 0, sizeof tmpa);
	memset(tmpb, 0, sizeof tmpb);
	for (int i = 0; i < 6; ++i)
		while (pq[i].size())
			pq[i].pop();
}
signed main() {
	cin.tie(0)->sync_with_stdio(0);
	cin >> n >> K >> L;
	for (int i = 1; i <= n; ++i)
		cin >> a[i], tmpa[i] = { a[i], i };
	for (int i = 1; i <= n; ++i)
		cin >> b[i], tmpb[i] = { b[i], i };
	stable_sort(tmpa + 1, tmpa + n + 1, greater<array<int, 2>>());
	stable_sort(tmpb + 1, tmpb + n + 1, greater<array<int, 2>>());
	for (int i = 1; i <= K; ++i) {
		sum += tmpa[i][0] + tmpb[i][0];
		st[tmpa[i][1]][0] = 1;
		st[tmpb[i][1]][1] = 1;
	}
	for (int i = 1; i <= n; ++i)
		if (st[i][0] == st[i][1] && st[i][0] == 1)
			--L, st[i][0] = st[i][1] = 2;
	for (int i = 1; i <= n; ++i)
		for (int j = 0; j < 6; ++j)
			trans[j](pq[j], i);
	while (L > 0) {
		for (int i = 0; i < 6; ++i)
			while (pq[i].size() && !ok[i](pq[i].top()[1]))
				pq[i].pop();
		nm = INT_MIN, op = 0;
		memset(ne, 0, sizeof ne);
		for (int i = 0; i < 4; ++i)
			work(i);
		upd();
		--L;
	}
	cout << sum << endl;
	return 0;
}