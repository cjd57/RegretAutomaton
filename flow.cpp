#include<iostream>
#include<vector>
#include<unordered_map>
#include<array>
#include<algorithm>
#include<numeric>
#include<set>
#include<functional>
#include<queue>
#include<bitset>
namespace mcmf {
	const int maxn = 300010;
	struct mcmf {
		struct edge {
			int v, cap, f, co;
			edge(int t, int c, int r, int co): v(t), cap(c), f(r), co(co) {}
		};
		std::vector<edge> ed;
		std::vector<int> g[maxn];
		int n, s, t, dis[maxn], h[maxn], pre[maxn];
		std::bitset<maxn> inq;
		void init() {
			for (auto& [t, c, f, co] : ed)
				f = 0;
			memset(dis, 0x3f, sizeof dis);
			memset(h, 0, sizeof h);
			memset(pre, 0, sizeof pre);
			inq = 0;
		}
		void operator()(const int &u, const int &v, const int &c, const int &co) {
			g[u].push_back(ed.size());
			ed.emplace_back(v, c, 0, co);
			g[v].push_back(ed.size());
			ed.emplace_back(u, 0, 0, -co);
		}
		void spfa(int s) {
			std::queue<int> q;
			q.push(s);
			inq = 0;
			inq[s] = 1;
			memset(h, 0x3f, sizeof h);
			h[s] = 0;
			while (q.size()) {
				int u = q.front();
				q.pop();
				inq[u] = 0;
				for (int i : g[u]) {
					auto& [v, cap, f, co] = ed[i];
					if (f < cap && h[v] > h[u] + co) {
						h[v] = h[u] + co;
						pre[v] = i;
						if (!inq[v]) {
							inq[v] = 1;
							q.push(v);
						}
					}
				}
			}
		}
		bool dijkstra(int s) {
			std::priority_queue<std::pair<int, int>,
			    std::vector<std::pair<int, int>>, std::greater<>> pq;
			memset(dis, 0x3f, sizeof dis);
			dis[s] = 0;
			pq.emplace(0, s);
			while (pq.size()) {
				auto [d, u] = pq.top();
				pq.pop();
				if (d > dis[u])
					continue;
				for (int i : g[u]) {
					auto [v, cap, f, co] = ed[i];
					co += h[u] - h[v];
					if (f < cap && dis[v] > dis[u] + co) {
						dis[v] = dis[u] + co;
						pre[v] = i;
						pq.emplace(dis[v], v);
					}
				}
			}
			return dis[t] < 0x3f3f3f3f;
		}
		std::pair<int, int> maxflow(int _s = -1, int _t = -1) {
			if (s >= 0)
				s = _s;
			if (t >= 0)
				t = _t;
			init();
			spfa(s);
			int flow = 0, cost = 0;
			while (dijkstra(s)) {
				int f = INT_MAX;
				for (int u = t; u != s; u = ed[pre[u] ^ 1].v) {
					f = std::min(f, ed[pre[u]].cap - ed[pre[u]].f);
				}
				for (int u = t; u != s; u = ed[pre[u] ^ 1].v) {
					ed[pre[u]].f += f;
					ed[pre[u] ^ 1].f -= f;
					cost += f * ed[pre[u]].co;
				}
				flow += f;
			}
			return {flow, cost};
		}
	};
}
using namespace std;
mcmf::mcmf mf;

int main() {
	ios::sync_with_stdio(false);
	cin.tie(0);
	int n, K, L;
	cin >> n >> K >> L;
	if (n > 100000)
		return -1;
	vector<int> a(n + 1), b(n + 1);
	for (int i = 1; i <= n; ++i)
		cin >> a[i];
	for (int i = 1; i <= n; ++i)
		cin >> b[i];
	int S = 0, T = 2 * n + 3;
	int p = 2 * n + 1, q = 2 * n + 2;
	int SS = 2 * n + 4;
	mf.n = SS + 1;
	mf(SS, S, K, 0);
	for (int i = 1; i <= n; ++i) {
		mf(S, i, 1, 0);
	}
	for (int i = 1; i <= n; ++i) {
		mf(n + i, T, 1, 0);
	}
	for (int i = 1; i <= n; ++i) {
		mf(i, n + i, 1, -(a[i] + b[i]));
	}
	for (int i = 1; i <= n; ++i) {
		mf(i, p, 1, -a[i]);
	}
	for (int i = 1; i <= n; ++i) {
		mf(q, n + i, 1, -b[i]);
	}
	mf(p, q, K - L, 0);
	auto [flow, cost] = mf.maxflow(SS, T);
	cout << -cost << '\n';
	return 0;
}