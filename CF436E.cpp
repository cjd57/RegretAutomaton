#include <bits/stdc++.h>
#include <numeric>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <vector>
#include <tuple>
#include <optional>
#include <ranges>
#include <climits>
#include <queue>
#include <array>

namespace RegretingGreed {
	struct Transform {
		int i, B, E;
	};
	using Regret = std::vector<Transform>;
	template<int D, int K, int (*_ita)(int, int), std::array<int, D>(*_C)(int)>
	struct RegretAutomaton {
		using ll = long long;
		int N, Delta;

		static int ita(const Transform &T) {
			return _ita(T.E, T.i) - _ita(T.B, T.i);
		}
		static int ita(const Regret &R) {
			int res = 0;
			for (const auto &T : R)
				res += ita(T);
			return res;
		}
		static std::array<int, D> C(const Transform &T) {
			auto e = _C(T.E);
			auto b = _C(T.B);
			std::array<int, D> res{};
			for (int i = 0; i < D; ++i)
				res[i] = e[i] - b[i];
			return res;
		}
		static std::array<int, D> C(const Regret &R) {
			std::array<int, D> res{};
			for (const auto &T : R) {
				auto delta = C(T);
				for (int i = 0; i < D; ++i)
					res[i] += delta[i];
			}
			return res;
		}

		struct TransformComparer {
			bool operator()(const Transform &Ta, const Transform &Tb) const {
				return std::tuple(ita(Ta), Ta.i, Ta.B, Ta.E) > std::tuple(ita(Tb), Tb.i, Tb.B, Tb.E);
			}
		};
		using Heap = std::priority_queue<Transform, std::vector<Transform>, TransformComparer>;

		std::unordered_map<ll, Heap> H;
		std::array<int, D> curVals{};

		struct Chooser {
			RegretAutomaton &th;
			Chooser(RegretAutomaton &fa) : th(fa) {}
			// cached arrays for the most recent candidate list
			std::vector<std::array<int, D>> cachedC;
			std::vector<ll> cachedI;

			std::vector<int> DP(std::vector<Transform> ca) {
				int M = (int)ca.size();
				// cache C and ita for candidates
				cachedC.assign(M, std::array<int, D> {});
				cachedI.assign(M, 0);
				for (int i = 0; i < M; ++i) {
					cachedC[i] = C(ca[i]);
					cachedI[i] = ita(ca[i]);
				}

				std::unordered_map<int, std::vector<int>> gr;
				gr.reserve(M);
				for (int i = 0; i < M; ++i)
					gr[ca[i].i].push_back(i);
				int maxDelta = K * th.Delta;
				int offset = maxDelta;
				int size = 2 * maxDelta + 1;
				std::vector<long long> dp(size, LLONG_MAX), ndp(size);

				// groups from gr
				std::vector<std::vector<int>> groups;
				groups.reserve(gr.size());
				for (auto &kv : gr)
					groups.push_back(kv.second);

				// parent pointers for backtracking: parent[g][d] = {prev_d, opIdx(-1 none)}
				std::vector<std::vector<std::pair<int, int>>> parent(groups.size(), std::vector<std::pair<int, int>>(size, { -2, -2 }));
				dp[offset] = 0;
				for (int g = 0; g < (int)groups.size(); ++g) {
					std::fill(ndp.begin(), ndp.end(), LLONG_MAX);
					for (int d = 0; d < size; ++d) {
						if (dp[d] == LLONG_MAX)
							continue;
						// option: take none from this group
						if (dp[d] < ndp[d]) {
							ndp[d] = dp[d];
							parent[g][d] = { d, -1 };
						}
						for (int opIdx : groups[g]) {
							int delta = cachedC[opIdx][0];
							int nd = d + delta;
							if (nd < 0 || nd >= size)
								continue;
							long long ncost = dp[d] + cachedI[opIdx];
							if (ncost < ndp[nd]) {
								ndp[nd] = ncost;
								parent[g][nd] = { d, opIdx };
							}
						}
					}
					dp.swap(ndp);
				}

				long long bestCost = LLONG_MAX;
				int bestD = -1;
				for (int d = offset; d < size; ++d) {
					if (dp[d] < bestCost) {
						bool ok = 0;
						for (int g = (int)parent.size() - 1; g >= 0; --g)
							if (parent[g][d].second != -1)
								ok = 1;
						if (!ok)
							continue;
						bestCost = dp[d];
						bestD = d;
					}
				}
				std::vector<int> bestCombo;
				if (bestD == -1)
					return bestCombo;
				// backtrack
				int curd = bestD;
				for (int g = (int)parent.size() - 1; g >= 0; --g) {
					auto pr = parent[g][curd];
					int prevd = pr.first;
					int op = pr.second;
					if (op != -1)
						bestCombo.push_back(op);
					curd = prevd;
				}
				std::reverse(bestCombo.begin(), bestCombo.end());
				return bestCombo;
			}

			struct Entry {
				ll code;
				ll cost;
				std::vector<int> combo;
				std::array<int, D> vec;
			};

			ll encode(const std::array<int, D> &vec) {
				ll base = (ll)2 * K * th.Delta + 1;
				ll code = 0;
				for (int d = 0; d < D; ++d) {
					code = code * base + (vec[d] + K * th.Delta);
				}
				return code;
			}

			std::array<int, D> decode(ll code) {
				ll base = (ll)2 * K * th.Delta + 1;
				std::array<int, D> vec{};
				for (int d = D - 1; d >= 0; --d) {
					vec[d] = (int)(code % base) - K * th.Delta;
					code /= base;
				}
				return vec;
			}

			std::vector<int> MITM(const std::vector<Transform> &ca) {
				int M = (int)ca.size();
				// cache C and ita
				std::vector<std::array<int, D>> cachedC(M);
				std::vector<ll> cachedI(M);
				for (int i = 0; i < M; ++i) {
					cachedC[i] = C(ca[i]);
					cachedI[i] = ita(ca[i]);
				}

				std::unordered_map<int, std::vector<int>> gr;
				for (int i = 0; i < M; ++i)
					gr[ca[i].i].push_back(i);
				std::vector<std::vector<int>> groups;
				groups.reserve(gr.size());
				for (auto &kv : gr)
					groups.push_back(kv.second);
				int G = groups.size();
				std::vector<int> order(G);
				iota(order.begin(), order.end(), 0);
				sort(order.begin(), order.end(), [&](int a, int b) {
					return groups[a].size() > groups[b].size();
				});
				std::vector<int> leftIds, rightIds;
				for (int i = 0; i < G; ++i) {
					if (i & 1)
						leftIds.push_back(order[i]);
					else
						rightIds.push_back(order[i]);
				}

				std::vector<Entry> left;
				left.reserve(1 << std::min((int)leftIds.size(), 10));
				auto dfsLeft = [&](auto &&dfsLeft, int idx, const std::array<int, D> &curVec, ll cost, std::vector<int> &combo) {
					if (idx == (int)leftIds.size()) {
						ll code = encode(curVec);
						left.push_back({ code, cost, combo, curVec });
						return;
					}
					int g = leftIds[idx];
					// don't take any from this group
					dfsLeft(dfsLeft, idx + 1, curVec, cost, combo);
					for (int opIdx : groups[g]) {
						std::array<int, D> nxt = curVec;
						for (int d = 0; d < D; ++d)
							nxt[d] += cachedC[opIdx][d];
						combo.push_back(opIdx);
						dfsLeft(dfsLeft, idx + 1, nxt, cost + cachedI[opIdx], combo);
						combo.pop_back();
					}
				};
				std::vector<int> emptyCombo;
				std::array<int, D> zero{};
				dfsLeft(dfsLeft, 0, zero, 0, emptyCombo);

				std::vector<Entry> right;
				right.reserve(1 << std::min((int)rightIds.size(), 10));
				auto dfsRight = [&](auto &&dfsRight, int idx, const std::array<int, D> &curVec, ll cost, std::vector<int> &combo) {
					if (idx == (int)rightIds.size()) {
						ll code = encode(curVec);
						right.push_back({ code, cost, combo, curVec });
						return;
					}
					int g = rightIds[idx];
					dfsRight(dfsRight, idx + 1, curVec, cost, combo);
					for (int opIdx : groups[g]) {
						std::array<int, D> nxt = curVec;
						for (int d = 0; d < D; ++d)
							nxt[d] += cachedC[opIdx][d];
						combo.push_back(opIdx);
						dfsRight(dfsRight, idx + 1, nxt, cost + cachedI[opIdx], combo);
						combo.pop_back();
					}
				};
				dfsRight(dfsRight, 0, zero, 0, emptyCombo);

				struct RightEntryInfo {
					int mainDelta;
					ll cost;
					int idx;
				};
				// prune: index by otherCode -> map mainDelta -> best RightEntryInfo
				std::unordered_map<ll, std::unordered_map<int, RightEntryInfo>> index;
				index.reserve(right.size() * 2 + 1);
				for (int i = 0; i < (int)right.size(); ++i) {
					const Entry &e = right[i];
					int mainDelta = e.vec[0];
					ll otherCode = 0;
					ll base = (ll)2 * K * th.Delta + 1;
					for (int d = 1; d < D; ++d)
						otherCode = otherCode * base + (e.vec[d] + K * th.Delta);
					auto &inner = index[otherCode];
					auto it = inner.find(mainDelta);
					if (it == inner.end() || e.cost < it->second.cost) {
						inner[mainDelta] = { mainDelta, e.cost, i };
					}
				}

				ll bestCost = LLONG_MAX;
				std::vector<int> bestCombo;
				for (const auto &l : left) {
					int lMain = l.vec[0];
					ll needOtherCode = 0;
					ll base = (ll)2 * K * th.Delta + 1;
					for (int d = 1; d < D; ++d) {
						int val = -l.vec[d] + K * th.Delta;
						needOtherCode = needOtherCode * base + val;
					}
					auto it = index.find(needOtherCode);
					if (it == index.end())
						continue;
					for (auto &p : it->second) {
						const RightEntryInfo &info = p.second;
						int totalMain = lMain + info.mainDelta;
						if (totalMain < 0)
							continue;
						ll cost = l.cost + info.cost;
						if (cost < bestCost) {
							bestCost = cost;
							bestCombo.clear();
							bestCombo.insert(bestCombo.end(), l.combo.begin(), l.combo.end());
							const std::vector<int> &rCombo = right[info.idx].combo;
							bestCombo.insert(bestCombo.end(), rCombo.begin(), rCombo.end());
						}
					}
				}
				return bestCombo;
			}

			std::vector<int> selectBestCombination(const std::vector<Transform> &candidates) {
				if (D == 1)
					return DP(candidates);
				else
					return MITM(candidates);
			}
		} sec;

		std::vector<Transform> getTopKOperations() {
			std::vector<Transform> candidates;
			candidates.reserve(H.size() * K);
			for (auto &kv : H) {
				auto &heap = kv.second;
				std::vector<Transform> temp;
				int cnt = 0;
				while (!heap.empty() && cnt < K) {
					Transform t = heap.top();
					heap.pop();
					if (t.B == states[t.i]) {
						candidates.push_back(t);
						temp.push_back(t);
						++cnt;
					}
				}
				for (const auto &t : temp)
					heap.push(t);
			}
			return candidates;
		}

		std::vector<int> states;
		std::function<std::vector<Transform>(int, const int &)> getTransitions;

		void insertTransitions(int id) {
			auto transList = getTransitions(id, states[id]);
			for (auto &t : transList) {
				if (t.B != states[id])
					continue;
				auto cvec = C(t);
				if (std::all_of(cvec.begin(), cvec.end(), [](int x) {
				return x == 0;
			}))
				continue;
				ll key = sec.encode(cvec);
				H[key].push(t);
			}
		}

		std::optional<ll> solve(int targetMain) {
			ll totalCost = 0;
			for (int i = 0; i < N; ++i)
				totalCost += _ita(states[i], i);
			while (true) {
				auto candidates = getTopKOperations();
				if (candidates.empty()) {
					if (curVals[0] >= targetMain)
						return totalCost;
					return std::optional<ll>();
				}

				auto bestCombo = sec.selectBestCombination(candidates);
				if (bestCombo.empty()) {
					if (curVals[0] >= targetMain)
						return totalCost;
					return std::optional<ll>();
				}
				bool hasCache = (sec.cachedI.size() == candidates.size() && sec.cachedC.size() == candidates.size());
				ll delcost = 0;
				for (int idx : bestCombo) {
					if (hasCache)
						delcost += sec.cachedI[idx];
					else
						delcost += ita(candidates[idx]);
				}
				if (curVals[0] >= targetMain && delcost >= 0)
					return totalCost;

				for (int idx : bestCombo) {
					const Transform &op = candidates[idx];
					int id = op.i;
					states[id] = op.E;
					if (hasCache) {
						for (int i = 0; i < D; ++i)
							curVals[i] += sec.cachedC[idx][i];
						totalCost += sec.cachedI[idx];
					} else {
						auto delta = C(op);
						for (int i = 0; i < D; ++i)
							curVals[i] += delta[i];
						totalCost += ita(op);
					}
					insertTransitions(id);
				}
			}
			return std::optional<ll>(totalCost);
		}

		RegretAutomaton(int n_,
		                const std::vector<int> &initStates,
		                std::function<std::vector<Transform>(int, const int &)> getTransitions_,
		                int maxDeltaAbs_)
			: N(n_), states(initStates), curVals{},
			  Delta(maxDeltaAbs_), getTransitions(getTransitions_),
			  sec(*this) {
			for (int i = 0; i < N; ++i) {
				insertTransitions(i);
				auto initDelta = _C(initStates[i]);
				for (int j = 0; j < D; ++j)
					curVals[j] += initDelta[j];
			}
		}
	};
}
using namespace std;
vector<int>a, c;

int ita(int u, int i) {
	if (u == 0)
		return 0;
	else if (u == 1)
		return a[i];
	else
		return c[i];
}

array<int, 1> C(int i) {
	return { i };
}

vector<RegretingGreed::Transform>getTran(int id, int state) {
	vector<RegretingGreed::Transform> res;
	for (int i : {
	            0, 1, 2
	        })
		if (state != i)
			res.push_back({ id, state, i });
	return res;
}

int main() {
	cin.tie(0)->sync_with_stdio(0);
	int n, w;
	cin >> n >> w;
	a.resize(n);
	c.resize(n);
	for (int i = 0; i < n; ++i)
		cin >> a[i] >> c[i];
	auto *ra = new RegretingGreed::RegretAutomaton<1, 1, ita, C>(n, vector<int>(n, 0), getTran, 3);
	cout << ra->solve(w).value() << endl;
	for (int i = 0; i < n; ++i)
		cout << ra->states[i];
	return 0;
}