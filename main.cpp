// A* Baseline (jarak) vs A* Modifikasi (waktu + kondisi jalan) - rute pejalan kaki ke halte
// Pakai: main [lat lon skenario]   skenario: 1 Normal, 2 Padat, 3 Hambatan berat, 4 Penutupan jalur
// Graf SIMULASI: 10 persimpangan buatan + 18 halte nyata (OSM). Harus identik dengan index.html.
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
using namespace std;

const double PI = 3.14159265358979323846, RB = 6371000.0;
const double V_NORMAL = 1.2;   // m/s pejalan kaki
const double V_MAKS   = 1.2;   // kecepatan tertinggi di graf (skenario hanya menurunkan) -> h admissible
const double FAKTOR   = 1.2;   // faktor jalur untuk konektor titik awal
const double INF = 1e18;

struct Node { string id, nama; double lat, lon; bool halte; };
struct Edge { int a, b; double d, v, p; bool tutup; };   // 1 edge dipakai kedua arah -> status selalu konsisten
vector<Node> N; vector<Edge> E; vector<vector<int>> adj; unordered_map<string,int> ix;

double hav(double la1, double lo1, double la2, double lo2) {
    auto r = [](double x) { return x * PI / 180.0; };
    double a = pow(sin(r(la2 - la1) / 2), 2) + cos(r(la1)) * cos(r(la2)) * pow(sin(r(lo2 - lo1) / 2), 2);
    return RB * 2 * atan2(sqrt(a), sqrt(1 - a));
}
void addN(string id, string nama, double la, double lo, bool h) {
    ix[id] = N.size(); N.push_back({id, nama, la, lo, h}); adj.push_back({});
}
void addE(string a, string b, double d) {
    int id = E.size(); E.push_back({ix.at(a), ix.at(b), d, V_NORMAL, 0, false});
    adj[E[id].a].push_back(id); adj[E[id].b].push_back(id);
}

void muat() {
    double J[10][2] = {{-6.3090,106.7910},{-6.3090,106.7945},{-6.3092,106.7975},{-6.3108,106.7905},{-6.3107,106.7940},
                       {-6.3107,106.7975},{-6.3120,106.7925},{-6.3122,106.7895},{-6.3127,106.7955},{-6.3132,106.7988}};
    for (int i = 0; i < 10; i++) addN("J" + to_string(i + 1), "Persimpangan J" + to_string(i + 1), J[i][0], J[i][1], false);
    struct H { const char* n; double la, lo; } hs[18] = {
        {"Pasar Pondok Labu",-6.3120408,106.7940991},{"SMPN 85",-6.3123693,106.7952284},
        {"Simpang Pinang 1 Pondok Labu",-6.3091279,106.7957296},{"Graha Werdatama Pondok Labu",-6.3091324,106.7947118},
        {"Taman DDN",-6.3125617,106.7963705},{"Jalan Pinang Raya",-6.3089474,106.7937556},
        {"Jalan Haji Ipin",-6.3110602,106.793585},{"Jalan Haji Muslim",-6.3090054,106.7936729},
        {"Jalan Pinang II Pondok Labu",-6.3104352,106.7966723},{"Sbr. Akper Fatmawati Pondok Labu",-6.3131017,106.79938},
        {"Simpang DDN Pondok Labu",-6.31264,106.79687},{"Sekolah Darussalam H. Ipin",-6.311351,106.79271},
        {"Al Izhar Pondok Labu 2",-6.311504,106.79104},{"Sbr. Sekolah Keperawatan YPDR",-6.311342,106.78859},
        {"Sekolah Keperawatan YPDR",-6.311307,106.78843},{"Al Izhar Pondok Labu 1",-6.3115787,106.7909388},
        {"Masjid At Taqwa H. Ipin",-6.3114796,106.7927906},{"Akper Fatmawati Pondok Labu",-6.313187,106.79931}};
    for (int i = 0; i < 18; i++) addN("H" + to_string(i + 1), hs[i].n, hs[i].la, hs[i].lo, true);
    struct Ed { const char *a, *b; double d; } es[] = {
        {"J1","J2",464},{"J2","J3",399},{"J1","J4",249},{"J2","J5",236},{"J3","J6",200},{"J4","J5",464},{"J5","J6",464},
        {"J4","J7",310},{"J4","J8",229},{"J5","J7",264},{"J6","J9",376},{"J7","J8",399},{"J7","J9",409},{"J9","J10",443},
        {"H1","J5",179},{"H2","J9",57},{"H3","J2",164},{"H4","J2",33},{"H5","J9",117},{"H6","J2",99},{"H7","J5",73},
        {"H8","J2",110},{"H9","J6",115},{"H10","J10",78},{"H11","J9",182},{"H12","J7",91},{"H13","J4",118},
        {"H14","J8",166},{"H15","J8",185},{"H16","J4",119},{"H17","J7",79},{"H18","J10",68}};
    for (auto& e : es) addE(e.a, e.b, e.d);
}

// titik awal menempel ke persimpangan terdekat (konektor tidak terkena skenario)
int tambahStart(double lat, double lon) {
    int bi = 0; double bd = INF;
    for (int i = 0; i < 10; i++) { double d = hav(lat, lon, N[i].lat, N[i].lon); if (d < bd) { bd = d; bi = i; } }
    addN("S", "Titik awal", lat, lon, false);
    addE("S", N[bi].id, bd * FAKTOR);
    if (bd > 1000) cout << "[PERINGATAN] Titik awal " << (int)bd << " m dari persimpangan terdekat: di luar area simulasi.\n";
    return ix["S"];
}

void setEdge(const char* a, const char* b, double v, double p, bool t) {
    int x = ix.at(a), y = ix.at(b);
    for (auto& e : E) if ((e.a == x && e.b == y) || (e.a == y && e.b == x)) { e.v = v; e.p = p; e.tutup = t; }
}
// 1) reset semua ke normal  2) ubah atribut ruas sesuai skenario (1 edge = kedua arah)
void skenario(int s) {
    for (auto& e : E) { e.v = V_NORMAL; e.p = 0; e.tutup = false; }
    const char* L[][2] = {{"J2","J5"},{"J5","J6"},{"J7","J9"},{"J4","J7"},{"H1","J5"},{"H7","J5"},{"H12","J7"},{"H17","J7"},{"H13","J4"},{"H2","J9"}};
    if (s == 2 || s == 3) for (auto& u : L) setEdge(u[0], u[1], s == 2 ? 0.8 : 0.6, s == 3 ? 45 : 0, false);
    if (s == 4) { setEdge("J1","J4",0,0,true); setEdge("J3","J6",0,0,true); setEdge("J4","J5",0,0,true); }
}

struct Hasil { bool ok = false; double g = INF; int expanded = 0; vector<int> path, edges; };

// mode 0 = A* baseline : g = jarak, h = haversine, abaikan kondisi jalan
// mode 1 = A* modifikasi: g = d/v + p, ruas tutup dilewati, h = haversine / V_MAKS
Hasil astar(int mode, int s, int t) {
    int n = N.size(); vector<double> g(n, INF); vector<int> pe(n, -1); vector<char> cl(n, 0);
    auto h = [&](int u) { double d = hav(N[u].lat, N[u].lon, N[t].lat, N[t].lon); return mode ? d / V_MAKS : d; };
    priority_queue<pair<double,int>, vector<pair<double,int>>, greater<pair<double,int>>> q;
    g[s] = 0; q.push({h(s), s}); Hasil r;
    while (!q.empty()) {
        int u = q.top().second; q.pop();
        if (cl[u]) continue;
        cl[u] = 1; r.expanded++;
        if (u == t) break;
        for (int id : adj[u]) {
            Edge& e = E[id]; if (mode && e.tutup) continue;
            int w = e.a == u ? e.b : e.a; if (cl[w]) continue;
            double c = mode ? e.d / e.v + e.p : e.d;
            if (g[u] + c < g[w]) { g[w] = g[u] + c; pe[w] = id; q.push({g[w] + h(w), w}); }
        }
    }
    if (g[t] >= INF / 2) return r;
    r.ok = true; r.g = g[t];
    for (int u = t; u != s;) { int id = pe[u]; r.edges.push_back(id); r.path.push_back(u); u = E[id].a == u ? E[id].b : E[id].a; }
    r.path.push_back(s); reverse(r.path.begin(), r.path.end()); reverse(r.edges.begin(), r.edges.end());
    return r;
}

struct Item { int t; Hasil h; double jarak = 0, waktu = 0; bool blok = false; };
// A* ke 18 halte, urut berdasarkan g algoritmanya; rute DINILAI pada kondisi skenario aktif (waktu nyata / TERHALANG)
vector<Item> cari(int mode, int s, int& totalExp) {
    vector<Item> v; totalExp = 0;
    for (int i = 0; i < (int)N.size(); i++) {
        if (!N[i].halte) continue;
        Hasil h = astar(mode, s, i); totalExp += h.expanded; if (!h.ok) continue;
        Item it; it.t = i; it.h = h;
        for (int id : h.edges) { Edge& e = E[id]; it.jarak += e.d; if (e.tutup) it.blok = true; else it.waktu += e.d / e.v + e.p; }
        v.push_back(it);
    }
    sort(v.begin(), v.end(), [](const Item& a, const Item& b) { return a.h.g != b.h.g ? a.h.g < b.h.g : a.t < b.t; });
    return v;
}

void tampil(const char* judul, int mode, int s) {
    int tot; auto v = cari(mode, s, tot);
    printf("\n== %s ==\n", judul);
    for (int k = 0; k < 3 && k < (int)v.size(); k++) {
        auto& it = v[k];
        printf("  #%d %s %s | jarak %.1f m | ", k + 1, N[it.t].id.c_str(), N[it.t].nama.c_str(), it.jarak);
        if (it.blok) printf("TERHALANG (lewat ruas tutup)\n"); else printf("waktu %.1f dtk (%.1f mnt)\n", it.waktu, it.waktu / 60);
        printf("      rute:"); for (int u : it.h.path) printf(" %s", N[u].id.c_str()); printf("\n");
    }
    auto t0 = chrono::steady_clock::now();
    for (int i = 0; i < 1000; i++) { int x; cari(mode, s, x); }
    double ms = chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count() / 1000;
    printf("  node dikembangkan (18 pencarian): %d | waktu eksekusi: %.4f ms\n", tot, ms);
}

int main(int argc, char** argv) {
    muat(); double lat = -6.3150, lon = 106.7930; int sk = 1;
    if (argc >= 4) { lat = atof(argv[1]); lon = atof(argv[2]); sk = atoi(argv[3]); }
    else {
        cout << "Latitude : "; cin >> lat; cout << "Longitude: "; cin >> lon;
        cout << "Skenario (1 Normal, 2 Padat, 3 Hambatan berat, 4 Penutupan jalur): "; cin >> sk;
    }
    if (!cin && argc < 4) { cout << "Input tidak valid.\n"; return 1; }
    if (sk < 1 || sk > 4) { cout << "Skenario harus 1-4.\n"; return 1; }
    int s = tambahStart(lat, lon);
    skenario(sk);
    tampil("A* BASELINE (urut jarak, dinilai pada kondisi skenario)", 0, s);
    tampil("A* MODIFIKASI (urut waktu, kondisi jalan dihitung)", 1, s);
    return 0;
}
