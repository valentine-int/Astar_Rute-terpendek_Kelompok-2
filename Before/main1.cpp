
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <cmath>
#include <algorithm>
#include <limits>
#include <iomanip>
#include <chrono>

using namespace std;

// ==========================================
// 1. KONSTANTA
// ==========================================

const double PI = 3.14159265358979323846;
const double RADIUS_BUMI = 6371000.0;

const double KECEPATAN_NORMAL = 1.2;
const double KECEPATAN_MAKS = 1.5;
const double FAKTOR_JALUR = 1.2;

const double INF = numeric_limits<double>::infinity();


// ==========================================
// 2. STRUKTUR DATA
// ==========================================

struct Node {
    string id;
    string nama;
    double lat;
    double lon;
    bool halte;
};

struct Edge {
    int tujuan;
    double jarak;
    double kecepatan;
    double penalti;
    bool tertutup;
};

struct HasilAStar {
    bool ditemukan = false;
    double totalBiaya = INF;
    double totalWaktu = 0;
    double totalJarak = 0;
    vector<int> jalur;
};

struct StatistikAStar {
    int jumlahNodeDikembangkan = 0;
    size_t maksimumPriorityQueue = 0;
};


// ==========================================
// 3. VARIABEL GRAF
// ==========================================

vector<Node> nodes;
vector<vector<Edge>> graf;

unordered_map<string, int> indeksNode;


// ==========================================
// 4. FUNGSI HAVERSINE
// ==========================================

double keRadian(double derajat) {
    return derajat * PI / 180.0;
}

double hitungJarak(
    double lat1, double lon1,
    double lat2, double lon2
) {
    double dLat = keRadian(lat2 - lat1);
    double dLon = keRadian(lon2 - lon1);

    lat1 = keRadian(lat1);
    lat2 = keRadian(lat2);

    double a =
        pow(sin(dLat / 2), 2) +
        cos(lat1) * cos(lat2) *
        pow(sin(dLon / 2), 2);

    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return RADIUS_BUMI * c;
}


// ==========================================
// 5. MENAMBAHKAN NODE
// ==========================================

int tambahNode(
    string id,
    string nama,
    double lat,
    double lon,
    bool halte
) {
    int indeks = nodes.size();

    nodes.push_back({id, nama, lat, lon, halte});
    graf.push_back({});
    indeksNode[id] = indeks;

    return indeks;
}


// ==========================================
// 6. MENAMBAHKAN EDGE DUA ARAH
// ==========================================

void tambahEdge(
    string idAwal,
    string idTujuan,
    double jarak,
    double kecepatan = KECEPATAN_NORMAL,
    double penalti = 0,
    bool tertutup = false
) {
    int awal = indeksNode.at(idAwal);
    int tujuan = indeksNode.at(idTujuan);

    graf[awal].push_back({
        tujuan, jarak, kecepatan, penalti, tertutup
    });

    graf[tujuan].push_back({
        awal, jarak, kecepatan, penalti, tertutup
    });
}


// ==========================================
// 7. MEMUAT DATA NODE
// ==========================================

void muatDataNode() {

    // Persimpangan simulasi

    tambahNode("J1", "Persimpangan J1", -6.3090, 106.7910, false);
    tambahNode("J2", "Persimpangan J2", -6.3090, 106.7945, false);
    tambahNode("J3", "Persimpangan J3", -6.3092, 106.7975, false);
    tambahNode("J4", "Persimpangan J4", -6.3108, 106.7905, false);
    tambahNode("J5", "Persimpangan J5", -6.3107, 106.7940, false);
    tambahNode("J6", "Persimpangan J6", -6.3107, 106.7975, false);
    tambahNode("J7", "Persimpangan J7", -6.3120, 106.7925, false);
    tambahNode("J8", "Persimpangan J8", -6.3122, 106.7895, false);
    tambahNode("J9", "Persimpangan J9", -6.3127, 106.7955, false);
    tambahNode("J10", "Persimpangan J10", -6.3132, 106.7988, false);

    // 18 halte dari OpenStreetMap

    tambahNode("H1", "Pasar Pondok Labu", -6.3120408, 106.7940991, true);
    tambahNode("H2", "SMPN 85", -6.3123693, 106.7952284, true);
    tambahNode("H3", "Simpang Pinang 1 Pondok Labu", -6.3091279, 106.7957296, true);
    tambahNode("H4", "Graha Werdatama Pondok Labu", -6.3091324, 106.7947118, true);
    tambahNode("H5", "Taman DDN", -6.3125617, 106.7963705, true);
    tambahNode("H6", "Jalan Pinang Raya", -6.3089474, 106.7937556, true);
    tambahNode("H7", "Jalan Haji Ipin", -6.3110602, 106.793585, true);
    tambahNode("H8", "Jalan Haji Muslim", -6.3090054, 106.7936729, true);
    tambahNode("H9", "Jalan Pinang II Pondok Labu", -6.3104352, 106.7966723, true);
    tambahNode("H10", "Sbr. Akper Fatmawati Pondok Labu", -6.3131017, 106.79938, true);
    tambahNode("H11", "Simpang DDN Pondok Labu", -6.31264, 106.79687, true);
    tambahNode("H12", "Sekolah Darussalam H. Ipin", -6.311351, 106.79271, true);
    tambahNode("H13", "Al Izhar Pondok Labu 2", -6.311504, 106.79104, true);
    tambahNode("H14", "Sbr. Sekolah Keperawatan YPDR", -6.311342, 106.78859, true);
    tambahNode("H15", "Sekolah Keperawatan YPDR", -6.311307, 106.78843, true);
    tambahNode("H16", "Al Izhar Pondok Labu 1", -6.3115787, 106.7909388, true);
    tambahNode("H17", "Masjid At Taqwa H. Ipin", -6.3114796, 106.7927906, true);
    tambahNode("H18", "Akper Fatmawati Pondok Labu", -6.313187, 106.79931, true);
}


// ==========================================
// 8. MEMBENTUK JARINGAN GRAF
// ==========================================

void muatDataEdge() {

    // Edge antar-persimpangan

    tambahEdge("J1", "J2", 464);
    tambahEdge("J2", "J3", 399);
    tambahEdge("J1", "J4", 249);
    tambahEdge("J2", "J5", 236);
    tambahEdge("J3", "J6", 200);
    tambahEdge("J4", "J5", 464);
    tambahEdge("J5", "J6", 464);
    tambahEdge("J4", "J7", 310);
    tambahEdge("J4", "J8", 229);
    tambahEdge("J5", "J7", 264);
    tambahEdge("J6", "J9", 376);
    tambahEdge("J7", "J8", 399);
    tambahEdge("J7", "J9", 409);
    tambahEdge("J9", "J10", 443);

    // Edge penghubung halte

    tambahEdge("H1", "J5", 179);
    tambahEdge("H2", "J9", 57);
    tambahEdge("H3", "J2", 164);
    tambahEdge("H4", "J2", 33);
    tambahEdge("H5", "J9", 117);
    tambahEdge("H6", "J2", 99);
    tambahEdge("H7", "J5", 73);
    tambahEdge("H8", "J2", 110);
    tambahEdge("H9", "J6", 115);
    tambahEdge("H10", "J10", 78);
    tambahEdge("H11", "J9", 182);
    tambahEdge("H12", "J7", 91);
    tambahEdge("H13", "J4", 118);
    tambahEdge("H14", "J8", 166);
    tambahEdge("H15", "J8", 185);
    tambahEdge("H16", "J4", 119);
    tambahEdge("H17", "J7", 79);
    tambahEdge("H18", "J10", 68);
}


// ==========================================
// 9. MENGUBAH KONDISI EDGE
// ==========================================

void ubahKondisiEdge(
    string idAwal,
    string idTujuan,
    double kecepatan,
    double penalti,
    bool tertutup
) {
    int awal = indeksNode.at(idAwal);
    int tujuan = indeksNode.at(idTujuan);

    for (Edge &edge : graf[awal]) {
        if (edge.tujuan == tujuan) {
            edge.kecepatan = kecepatan;
            edge.penalti = penalti;
            edge.tertutup = tertutup;
        }
    }

    for (Edge &edge : graf[tujuan]) {
        if (edge.tujuan == awal) {
            edge.kecepatan = kecepatan;
            edge.penalti = penalti;
            edge.tertutup = tertutup;
        }
    }
}


// ==========================================
// 10. MENERAPKAN SKENARIO
// ==========================================

void terapkanSkenario(int pilihan) {

    // Reset seluruh edge ke kondisi normal

    for (auto &daftarEdge : graf) {
        for (Edge &edge : daftarEdge) {
            edge.kecepatan = KECEPATAN_NORMAL;
            edge.penalti = 0;
            edge.tertutup = false;
        }
    }

    if (pilihan == 1) {
        cout << "\nSkenario: Normal\n";
    }
    else if (pilihan == 2) {
        cout << "\nSkenario: Padat\n";

        ubahKondisiEdge("J2", "J5", 0.8, 0, false);
        ubahKondisiEdge("J5", "J6", 0.8, 0, false);
        ubahKondisiEdge("J7", "J9", 0.8, 0, false);
    }
    else if (pilihan == 3) {
        cout << "\nSkenario: Hambatan Berat\n";

        ubahKondisiEdge("J2", "J5", 0.6, 45, false);
        ubahKondisiEdge("J5", "J6", 0.6, 45, false);
        ubahKondisiEdge("J7", "J9", 0.6, 45, false);
    }
    else if (pilihan == 4) {
        cout << "\nSkenario: Penutupan Jalur\n";

        ubahKondisiEdge("J4", "J5", 0, 0, true);
        ubahKondisiEdge("J6", "J9", 0, 0, true);
    }
}


// ==========================================
// 11. FUNGSI HEURISTIC
// ==========================================

// Heuristic A* modifikasi: estimasi waktu

double hitungHeuristic(int awal, int tujuan) {

    double jarak = hitungJarak(
        nodes[awal].lat,
        nodes[awal].lon,
        nodes[tujuan].lat,
        nodes[tujuan].lon
    );

    return jarak / KECEPATAN_MAKS;
}

// Heuristic A* baseline: estimasi jarak

double hitungHeuristicJarak(int awal, int tujuan) {

    return hitungJarak(
        nodes[awal].lat,
        nodes[awal].lon,
        nodes[tujuan].lat,
        nodes[tujuan].lon
    );
}


// ==========================================
// 12. ALGORITMA A* MODIFIKASI
// MENCARI WAKTU TEMPUH TERCEPAT
// ==========================================

HasilAStar jalankanAStar(
    int awal,
    int tujuan,
    StatistikAStar* statistik = nullptr
) {
    int jumlahNode = nodes.size();

    if (statistik != nullptr) {
        *statistik = {};
    }

    vector<double> gScore(jumlahNode, INF);
    vector<int> parent(jumlahNode, -1);

    priority_queue<
        pair<double, int>,
        vector<pair<double, int>>,
        greater<pair<double, int>>
    > antrian;

    gScore[awal] = 0;
    antrian.push({hitungHeuristic(awal, tujuan), awal});

    if (statistik != nullptr) {
        statistik->maksimumPriorityQueue = antrian.size();
    }

    while (!antrian.empty()) {

        int sekarang = antrian.top().second;
        double nilaiF = antrian.top().first;
        antrian.pop();

        if (nilaiF > gScore[sekarang] +
            hitungHeuristic(sekarang, tujuan) + 1e-9) {
            continue;
        }

        if (sekarang == tujuan) {
            break;
        }

        if (statistik != nullptr) {
            statistik->jumlahNodeDikembangkan++;
        }

        for (const Edge &edge : graf[sekarang]) {

            if (edge.tertutup || edge.kecepatan <= 0) {
                continue;
            }

            int tetangga = edge.tujuan;

            double waktuEdge =
                edge.jarak / edge.kecepatan + edge.penalti;

            double gBaru = gScore[sekarang] + waktuEdge;

            if (gBaru < gScore[tetangga]) {

                gScore[tetangga] = gBaru;
                parent[tetangga] = sekarang;

                double fBaru =
                    gBaru + hitungHeuristic(tetangga, tujuan);

                antrian.push({fBaru, tetangga});

                if (statistik != nullptr) {
                    statistik->maksimumPriorityQueue =
                        max(
                            statistik->maksimumPriorityQueue,
                            antrian.size()
                        );
                }
            }
        }
    }

    HasilAStar hasil;

    if (gScore[tujuan] == INF) {
        return hasil;
    }

    hasil.ditemukan = true;
    hasil.totalBiaya = gScore[tujuan];
    hasil.totalWaktu = gScore[tujuan];

    int sekarang = tujuan;

    while (sekarang != -1) {
        hasil.jalur.push_back(sekarang);
        sekarang = parent[sekarang];
    }

    reverse(hasil.jalur.begin(), hasil.jalur.end());

    // Hitung total jarak jalur

    for (int i = 1; i < (int)hasil.jalur.size(); i++) {

        int dari = hasil.jalur[i - 1];
        int ke = hasil.jalur[i];

        for (const Edge &edge : graf[dari]) {
            if (edge.tujuan == ke) {
                hasil.totalJarak += edge.jarak;
                break;
            }
        }
    }

    return hasil;
}


// ==========================================
// 13. ALGORITMA A* BASELINE
// MENCARI JARAK TERPENDEK
// ==========================================

HasilAStar jalankanAStarBaseline(
    int awal,
    int tujuan,
    StatistikAStar* statistik = nullptr
) {
    int jumlahNode = nodes.size();

    if (statistik != nullptr) {
        *statistik = {};
    }

    vector<double> gScore(jumlahNode, INF);
    vector<int> parent(jumlahNode, -1);

    priority_queue<
        pair<double, int>,
        vector<pair<double, int>>,
        greater<pair<double, int>>
    > antrian;

    gScore[awal] = 0;
    antrian.push({hitungHeuristicJarak(awal, tujuan), awal});

    if (statistik != nullptr) {
        statistik->maksimumPriorityQueue = antrian.size();
    }

    while (!antrian.empty()) {

        int sekarang = antrian.top().second;
        double nilaiF = antrian.top().first;
        antrian.pop();

        if (nilaiF > gScore[sekarang] +
            hitungHeuristicJarak(sekarang, tujuan) + 1e-9) {
            continue;
        }

        if (sekarang == tujuan) {
            break;
        }

        if (statistik != nullptr) {
            statistik->jumlahNodeDikembangkan++;
        }

        for (const Edge &edge : graf[sekarang]) {

            int tetangga = edge.tujuan;

            // Baseline hanya memperhitungkan jarak.
            // Kondisi kecepatan, penalti, dan penutupan diabaikan.

            double gBaru = gScore[sekarang] + edge.jarak;

            if (gBaru < gScore[tetangga]) {

                gScore[tetangga] = gBaru;
                parent[tetangga] = sekarang;

                double fBaru =
                    gBaru + hitungHeuristicJarak(tetangga, tujuan);

                antrian.push({fBaru, tetangga});

                if (statistik != nullptr) {
                    statistik->maksimumPriorityQueue =
                        max(
                            statistik->maksimumPriorityQueue,
                            antrian.size()
                        );
                }
            }
        }
    }

    HasilAStar hasil;

    if (gScore[tujuan] == INF) {
        return hasil;
    }

    hasil.ditemukan = true;
    hasil.totalBiaya = gScore[tujuan];
    hasil.totalJarak = gScore[tujuan];

    // Estimasi waktu hanya untuk tampilan baseline

    hasil.totalWaktu = hasil.totalJarak / KECEPATAN_NORMAL;

    int sekarang = tujuan;

    while (sekarang != -1) {
        hasil.jalur.push_back(sekarang);
        sekarang = parent[sekarang];
    }

    reverse(hasil.jalur.begin(), hasil.jalur.end());

    return hasil;
}


// ==========================================
// 14. MENAMBAHKAN TITIK AWAL PENGGUNA
// ==========================================

int tambahTitikAwal(double lat, double lon) {

    int indeksAwal = tambahNode(
        "S",
        "Titik Awal Pengguna",
        lat,
        lon,
        false
    );

    double jarakTerdekat = INF;
    int indeksPersimpangan = -1;

    for (int i = 0; i < indeksAwal; i++) {

        if (nodes[i].halte) {
            continue;
        }

        double jarak = hitungJarak(
            lat, lon,
            nodes[i].lat, nodes[i].lon
        );

        if (jarak < jarakTerdekat) {
            jarakTerdekat = jarak;
            indeksPersimpangan = i;
        }
    }

    double jarakSimulasi = jarakTerdekat * FAKTOR_JALUR;

    graf[indeksAwal].push_back({
        indeksPersimpangan,
        jarakSimulasi,
        KECEPATAN_NORMAL,
        0,
        false
    });

    graf[indeksPersimpangan].push_back({
        indeksAwal,
        jarakSimulasi,
        KECEPATAN_NORMAL,
        0,
        false
    });

    cout << fixed << setprecision(2);

    cout << "\nTitik awal terhubung ke: "
         << nodes[indeksPersimpangan].id << endl;

    cout << "Jarak konektor simulasi: "
         << jarakSimulasi << " meter\n";

    return indeksAwal;
}


// ==========================================
// 15. MENAMPILKAN RUTE
// ==========================================

void tampilkanRute(const vector<int> &jalur) {

    for (int i = 0; i < (int)jalur.size(); i++) {

        cout << nodes[jalur[i]].id;

        if (i < (int)jalur.size() - 1) {
            cout << " -> ";
        }
    }

    cout << endl;
}


// ==========================================
// 16. MENCARI 3 HALTE JARAK TERPENDEK
// A* BASELINE
// ==========================================

void cariTigaHalteBaseline(int awal) {

    vector<pair<int, HasilAStar>> hasilSemua;

    for (int i = 0; i < (int)nodes.size(); i++) {

        if (!nodes[i].halte) {
            continue;
        }

        HasilAStar hasil = jalankanAStarBaseline(awal, i);

        if (hasil.ditemukan) {
            hasilSemua.push_back({i, hasil});
        }
    }

    sort(
        hasilSemua.begin(),
        hasilSemua.end(),
        [](const auto &a, const auto &b) {
            return a.second.totalBiaya < b.second.totalBiaya;
        }
    );

    cout << "\n====================================\n";
    cout << "HASIL A* BASELINE\n";
    cout << "3 HALTE DENGAN JARAK TERPENDEK\n";
    cout << "====================================\n";

    int jumlah = min(3, (int)hasilSemua.size());

    for (int i = 0; i < jumlah; i++) {

        int indeksHalte = hasilSemua[i].first;
        HasilAStar hasil = hasilSemua[i].second;

        cout << "\nPeringkat " << i + 1 << endl;
        cout << "Kode halte  : " << nodes[indeksHalte].id << endl;
        cout << "Nama halte  : " << nodes[indeksHalte].nama << endl;
        cout << "Jarak       : " << hasil.totalJarak << " meter\n";
        cout << "Estimasi    : " << hasil.totalWaktu / 60
             << " menit (kecepatan normal)\n";
        cout << "Rute        : ";
        tampilkanRute(hasil.jalur);
    }

    cout << "\nJumlah halte yang dapat dijangkau: "
         << hasilSemua.size() << " dari 18\n";
}


// ==========================================
// 17. MENCARI 3 HALTE WAKTU TERCEPAT
// A* MODIFIKASI
// ==========================================

void cariTigaHalteTercepat(int awal) {

    vector<pair<int, HasilAStar>> hasilSemua;

    for (int i = 0; i < (int)nodes.size(); i++) {

        if (!nodes[i].halte) {
            continue;
        }

        HasilAStar hasil = jalankanAStar(awal, i);

        if (hasil.ditemukan) {
            hasilSemua.push_back({i, hasil});
        }
    }

    sort(
        hasilSemua.begin(),
        hasilSemua.end(),
        [](const auto &a, const auto &b) {
            return a.second.totalBiaya < b.second.totalBiaya;
        }
    );

    cout << "\n====================================\n";
    cout << "HASIL A* MODIFIKASI\n";
    cout << "3 HALTE DENGAN WAKTU TERCEPAT\n";
    cout << "====================================\n";

    int jumlah = min(3, (int)hasilSemua.size());

    for (int i = 0; i < jumlah; i++) {

        int indeksHalte = hasilSemua[i].first;
        HasilAStar hasil = hasilSemua[i].second;

        cout << "\nPeringkat " << i + 1 << endl;
        cout << "Kode halte  : " << nodes[indeksHalte].id << endl;
        cout << "Nama halte  : " << nodes[indeksHalte].nama << endl;
        cout << "Jarak       : " << hasil.totalJarak << " meter\n";
        cout << "Waktu       : " << hasil.totalWaktu << " detik\n";
        cout << "Estimasi    : " << hasil.totalWaktu / 60 << " menit\n";
        cout << "Rute        : ";
        tampilkanRute(hasil.jalur);
    }

    cout << "\nJumlah halte yang dapat dijangkau: "
         << hasilSemua.size() << " dari 18\n";
}


// ==========================================
// 18. PENGUJIAN SELURUH HALTE
// ==========================================

void ujiSeluruhHalte(int awal) {

    cout << "\n====================================\n";
    cout << "PENGUJIAN SELURUH 18 HALTE\n";
    cout << "A* MODIFIKASI\n";
    cout << "====================================\n";

    int jumlahTerjangkau = 0;

    for (int i = 0; i < (int)nodes.size(); i++) {

        if (!nodes[i].halte) {
            continue;
        }

        HasilAStar hasil = jalankanAStar(awal, i);

        cout << "\nHalte: " << nodes[i].id
             << " - " << nodes[i].nama << endl;

        if (!hasil.ditemukan) {
            cout << "Status: Tidak dapat dijangkau\n";
            continue;
        }

        jumlahTerjangkau++;

        cout << "Jarak: " << hasil.totalJarak << " meter\n";
        cout << "Waktu: " << hasil.totalWaktu << " detik\n";
        cout << "Rute: ";
        tampilkanRute(hasil.jalur);
    }

    cout << "\nJumlah halte yang dapat dijangkau: "
         << jumlahTerjangkau << " dari 18\n";
}


// ==========================================
// 19. PENGUJIAN KINERJA A*
// ==========================================

void ujiKinerjaAStar(int awal) {

    const int JUMLAH_PENGULANGAN = 1000;

    vector<int> daftarHalte;

    for (int i = 0; i < (int)nodes.size(); i++) {
        if (nodes[i].halte) {
            daftarHalte.push_back(i);
        }
    }

    long long totalNodeBaseline = 0;
    size_t maksimumQueueBaseline = 0;

    // Pengujian A* baseline

    auto mulaiBaseline = chrono::steady_clock::now();

    for (int pengulangan = 0;
         pengulangan < JUMLAH_PENGULANGAN;
         pengulangan++) {

        for (int tujuan : daftarHalte) {

            StatistikAStar statistik;

            HasilAStar hasil = jalankanAStarBaseline(
                awal, tujuan, &statistik
            );

            totalNodeBaseline += statistik.jumlahNodeDikembangkan;

            maksimumQueueBaseline = max(
                maksimumQueueBaseline,
                statistik.maksimumPriorityQueue
            );
        }
    }

    auto selesaiBaseline = chrono::steady_clock::now();

    double waktuBaseline =
        chrono::duration<double, milli>(
            selesaiBaseline - mulaiBaseline
        ).count();

    double rataRataBaseline =
        waktuBaseline / JUMLAH_PENGULANGAN;

    double rataNodeBaseline =
        (double)totalNodeBaseline / JUMLAH_PENGULANGAN;


    // Pengujian A* modifikasi

    long long totalNodeModifikasi = 0;
    size_t maksimumQueueModifikasi = 0;

    auto mulaiModifikasi = chrono::steady_clock::now();

    for (int pengulangan = 0;
         pengulangan < JUMLAH_PENGULANGAN;
         pengulangan++) {

        for (int tujuan : daftarHalte) {

            StatistikAStar statistik;

            HasilAStar hasil = jalankanAStar(
                awal, tujuan, &statistik
            );

            totalNodeModifikasi += statistik.jumlahNodeDikembangkan;

            maksimumQueueModifikasi = max(
                maksimumQueueModifikasi,
                statistik.maksimumPriorityQueue
            );
        }
    }

    auto selesaiModifikasi = chrono::steady_clock::now();

    double waktuModifikasi =
        chrono::duration<double, milli>(
            selesaiModifikasi - mulaiModifikasi
        ).count();

    double rataRataModifikasi =
        waktuModifikasi / JUMLAH_PENGULANGAN;

    double rataNodeModifikasi =
        (double)totalNodeModifikasi / JUMLAH_PENGULANGAN;


    // Menghitung perbedaan waktu

    double selisihWaktu =
        rataRataModifikasi - rataRataBaseline;

    double persentasePerubahan = 0;

    if (rataRataBaseline > 0) {
        persentasePerubahan =
            (selisihWaktu / rataRataBaseline) * 100;
    }


    // Menampilkan hasil

    cout << fixed << setprecision(6);

    cout << "\n====================================\n";
    cout << "HASIL PENGUJIAN KINERJA A*\n";
    cout << "====================================\n";

    cout << "Jumlah pengulangan : "
         << JUMLAH_PENGULANGAN << endl;

    cout << "Jumlah halte       : "
         << daftarHalte.size() << endl;

    cout << "\n--- A* BASELINE ---\n";

    cout << "Rata-rata waktu eksekusi : "
         << rataRataBaseline << " ms\n";

    cout << "Rata-rata node dikembangkan : "
         << rataNodeBaseline << endl;

    cout << "Maksimum priority queue : "
         << maksimumQueueBaseline << endl;

    cout << "\n--- A* MODIFIKASI ---\n";

    cout << "Rata-rata waktu eksekusi : "
         << rataRataModifikasi << " ms\n";

    cout << "Rata-rata node dikembangkan : "
         << rataNodeModifikasi << endl;

    cout << "Maksimum priority queue : "
         << maksimumQueueModifikasi << endl;

    cout << "\n--- PERBANDINGAN ---\n";

    cout << "Selisih waktu : "
         << selisihWaktu << " ms\n";

    cout << "Perubahan waktu : "
         << persentasePerubahan << "%\n";

    if (selisihWaktu > 0) {
        cout << "A* modifikasi membutuhkan waktu lebih lama.\n";
    }
    else if (selisihWaktu < 0) {
        cout << "A* modifikasi membutuhkan waktu lebih singkat.\n";
    }
    else {
        cout << "Kedua algoritma memiliki waktu yang sama.\n";
    }

    cout << "\nCatatan: Pengujian dilakukan untuk seluruh halte "
         << "sebanyak 1000 pengulangan.\n";

    cout << "Ukuran priority queue merupakan indikator "
         << "penggunaan memori, bukan pengukuran RAM aktual.\n";
}


// ==========================================
// 20. PROGRAM UTAMA
// ==========================================

int main() {

    cout << "====================================\n";
    cout << "SISTEM PENCARIAN HALTE TERDEKAT\n";
    cout << "PERBANDINGAN ALGORITMA A*\n";
    cout << "====================================\n";

    // Memuat data graf

    muatDataNode();
    muatDataEdge();

    cout << "\nJumlah node tetap: " << nodes.size() << endl;
    cout << "Jumlah edge dua arah: 32\n";


    // Input koordinat pengguna

    double lat, lon;

    cout << "\nMasukkan latitude titik awal: ";
    cin >> lat;

    cout << "Masukkan longitude titik awal: ";
    cin >> lon;


    // Input skenario

    int pilihan;

    cout << "\nPilih skenario kondisi jalan:\n";
    cout << "1. Normal\n";
    cout << "2. Padat\n";
    cout << "3. Hambatan berat\n";
    cout << "4. Penutupan jalur\n";

    cout << "Pilihan: ";
    cin >> pilihan;

    if (cin.fail() || pilihan < 1 || pilihan > 4) {
        cout << "\nInput tidak valid.\n";
        return 1;
    }


    // Terapkan skenario

    terapkanSkenario(pilihan);


    // Tambahkan titik awal pengguna

    int awal = tambahTitikAwal(lat, lon);


    // Jalankan A* baseline

    cariTigaHalteBaseline(awal);


    // Jalankan A* modifikasi

    cariTigaHalteTercepat(awal);


    // Pengujian seluruh halte

    ujiSeluruhHalte(awal);


    // Pengujian kinerja algoritma

    ujiKinerjaAStar(awal);


    return 0;
}
