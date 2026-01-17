// connection_check.cpp
// Small utility to check whether three nodes are connected "in series"
// (i.e. there exists a simple path that visits all three nodes without repeating vertices).

#include <bits/stdc++.h>
using namespace std;

// Returns true if there exists a simple path (no repeated nodes) that visits
// all three targets a,b,c (in any order). Graph is undirected and given as adjacency list.

bool dfs_has_all_targets(int u, const vector<vector<int>>& adj, vector<char>& visited, int mask, const vector<int>& targetIndexMask, int fullMask)
{
    // mark target visit
    mask |= targetIndexMask[u];
    if (mask == fullMask) return true;

    for (int v : adj[u])
    {
        if (!visited[v])
        {
            visited[v] = 1;
            if (dfs_has_all_targets(v, adj, visited, mask, targetIndexMask, fullMask))
                return true;
            visited[v] = 0;
        }
    }
    return false;
}

bool has_series_path(const vector<vector<int>>& adj, int a, int b, int c)
{
    int n = (int)adj.size();
    if (a<0||a>=n||b<0||b>=n||c<0||c>=n) return false;

    // Assign each node a bitmask (0 if not a target, otherwise 1<<i)
    vector<int> targetIdx(n, -1);
    vector<int> targetIndexMask(n, 0);
    vector<int> targets = {a,b,c};
    for (int i=0;i<3;++i) targetIdx[targets[i]] = i;
    for (int i=0;i<n;++i) if (targetIdx[i] >= 0) targetIndexMask[i] = (1<<targetIdx[i]);

    int fullMask = (1<<3)-1; // 0b111

    // Try starting DFS from each of the three target nodes
    for (int start : targets)
    {
        vector<char> visited(n, 0);
        visited[start] = 1;
        if (dfs_has_all_targets(start, adj, visited, 0, targetIndexMask, fullMask))
            return true;
    }
    return false;
}

// --- Small helper to build undirected graph from edge list ---
vector<vector<int>> buildGraph(int n, const vector<pair<int,int>>& edges)
{
    vector<vector<int>> adj(n);
    for (auto &e : edges)
    {
        int u=e.first, v=e.second;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    return adj;
}

// --- Simple tests ---
void runTests()
{
    {
        // linear chain 0-1-2-3, check nodes 0,2,3 -> path exists 0-1-2-3
        auto adj = buildGraph(4, {{0,1},{1,2},{2,3}});
        cout << "Test1 (expected 1): " << has_series_path(adj, 0,2,3) << "\n";
    }
    {
        // triangle 0-1-2-0, nodes 0,1,2 -> path exists (0-1-2)
        auto adj = buildGraph(3, {{0,1},{1,2},{2,0}});
        cout << "Test2 (expected 1): " << has_series_path(adj, 0,1,2) << "\n";
    }
    {
        // two components: 0-1   2-3, nodes 0,1,3 -> no
        auto adj = buildGraph(4, {{0,1},{2,3}});
        cout << "Test3 (expected 0): " << has_series_path(adj, 0,1,3) << "\n";
    }
    {
        // graph with cycle, but to visit three nodes might require revisiting nodes - can't
        // Example: 0-1-2, 1-3; check nodes 0,2,3 => path 0-1-3 then cannot reach 2 without repeating 1.
        auto adj = buildGraph(4, {{0,1},{1,2},{1,3}});
        cout << "Test4 (expected 0): " << has_series_path(adj, 0,2,3) << "\n";
    }
    {
        // longer graph where intermediates allowed: 0-4-1-5-2 (a path connecting 0..1..2)
        auto adj = buildGraph(6, {{0,4},{4,1},{1,5},{5,2}});
        cout << "Test5 (expected 1): " << has_series_path(adj, 0,1,2) << "\n";
    }
}

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        cout << "Running built-in tests...\n";
        runTests();
        return 0;
    }

    // Example CLI usage: first arg = n (nodes), then m edges, then pairs u v, then three targets
    // For simplicity we expect a small custom input: n m edges... a b c
    // e.g.: ./connection_check 4 3 0 1 1 2 2 3 0 2 3

    int idx = 1;
    int n = stoi(argv[idx++]);
    int m = stoi(argv[idx++]);
    vector<pair<int,int>> edges;
    for (int i=0;i<m;++i)
    {
        int u = stoi(argv[idx++]);
        int v = stoi(argv[idx++]);
        edges.emplace_back(u,v);
    }
    int a = stoi(argv[idx++]);
    int b = stoi(argv[idx++]);
    int c = stoi(argv[idx++]);

    auto adj = buildGraph(n, edges);
    bool ok = has_series_path(adj, a,b,c);
    cout << (ok ? "CONNECTED_IN_SERIES" : "NOT_CONNECTED_IN_SERIES") << "\n";
    return 0;
}
