#include "BranchBenchmark.h"
#include <chrono>

using Clock = std::chrono::high_resolution_clock;
using Us    = std::chrono::duration<double, std::micro>;

BranchBenchmark::BranchBenchmark(Branch& branch, int N, int M)
    : _branch(branch), _N(N), _M(M), _resultCount(0) {}

void BranchBenchmark::run() {
    _resultCount = 0;
    runSearchBenchmark();
    runInsertBenchmark();
    runRemoveBenchmark();
}

// ── Keys ─────────────────────────────────────────────────────────────────────

std::string BranchBenchmark::randomName() const {
    const ListNode* node = _branch.rawList().getHead();
    return node ? node->data.name : "";
}

std::string BranchBenchmark::firstSortName() const {
    const SortedListNode* node = _branch.rawSortedList().getHead();
    return node ? node->data.name : "";
}

std::string BranchBenchmark::lastListName() const {
    const ListNode* node = _branch.rawList().getHead();
    const ListNode* last = node;
    while (node) { last = node; node = node->next; }
    return last ? last->data.name : "";
}

std::string BranchBenchmark::missName() const {
    return "ZZZZZ_NO_EXISTE_9999";
}

std::string BranchBenchmark::randomBarcode() const {
    const ListNode* node = _branch.rawList().getHead();
    return node ? node->data.barcode : "";
}

Product BranchBenchmark::benchProduct() const {
    return Product("BenchBranchProd", "BENCH_BR_000", "Benchmark",
                   "2099-01-01", "BenchMarca", 1.0, 1);
}

// ── Search ────────────────────────────────────────────────────────────────────

void BranchBenchmark::runSearchBenchmark() {
    std::string rName    = randomName();
    std::string fName    = firstSortName();
    std::string lName    = lastListName();
    std::string mName    = missName();
    std::string rBarcode = randomBarcode();

    if (rName.empty()) return;

    // ── Por nombre: LinkedList, SortedList, AVL ───────────────────────────────
    struct Caso { const char* label; std::string key; };
    Caso casos[] = {
        { "hit_random", rName  },
        { "miss       ", mName  },
        { "hit_first  ", fName  },
        { "hit_last   ", lName  }
    };

    struct Modo {
        const char* name;
        std::function<void(const std::string&)> fn;
    };
    Modo modos[] = {
        { "LinkedList", [&](const std::string& k){ _branch.rawList().searchByName(k); } },
        { "SortedList", [&](const std::string& k){ _branch.rawSortedList().searchSequential(k); } },
        { "AVL",        [&](const std::string& k){ _branch.rawAVL().search(k); } }
    };

    for (auto& caso : casos) {
        for (auto& modo : modos) {
            double total = 0, minT = 1e18, maxT = 0;
            for (int rep = 0; rep < _M; ++rep) {
                auto t0 = Clock::now();
                for (int n = 0; n < _N; ++n) modo.fn(caso.key);
                auto t1 = Clock::now();
                double e = Us(t1 - t0).count();
                total += e;
                if (e < minT) minT = e;
                if (e > maxT) maxT = e;
            }
            auto& r = _results[_resultCount++];
            r.operation = "busqueda";
            r.structure = modo.name;
            r.caseType  = caso.label;
            r.avgTimeUs = total / (_M * _N);
            r.minTimeUs = minT  / _N;
            r.maxTimeUs = maxT  / _N;
            r.N = _N; r.M = _M;
        }
    }

    // ── Por barcode: HashTable ─────────────────────────────────────────────────
    if (!rBarcode.empty()) {
        Caso bcCasos[] = {
            { "hit_random", rBarcode         },
            { "miss       ", "MISS_BC_99999" }
        };
        for (auto& caso : bcCasos) {
            double total = 0, minT = 1e18, maxT = 0;
            for (int rep = 0; rep < _M; ++rep) {
                auto t0 = Clock::now();
                for (int n = 0; n < _N; ++n) _branch.rawHash().search(caso.key);
                auto t1 = Clock::now();
                double e = Us(t1 - t0).count();
                total += e;
                if (e < minT) minT = e;
                if (e > maxT) maxT = e;
            }
            auto& r = _results[_resultCount++];
            r.operation = "busqueda";
            r.structure = "HashTable";
            r.caseType  = caso.label;
            r.avgTimeUs = total / (_M * _N);
            r.minTimeUs = minT  / _N;
            r.maxTimeUs = maxT  / _N;
            r.N = _N; r.M = _M;
        }
    }

    // ── Por fecha: BTree ──────────────────────────────────────────────────────
    {
        std::string rDate, firstDate, lastDate;
        const std::string mDate = "1900-01-01";
        const ListNode* nd = _branch.rawList().getHead();
        if (nd) {
            rDate = firstDate = lastDate = nd->data.expiry_date;
            while (nd) {
                if (nd->data.expiry_date < firstDate) firstDate = nd->data.expiry_date;
                if (nd->data.expiry_date > lastDate)  lastDate  = nd->data.expiry_date;
                nd = nd->next;
            }
        }
        if (!rDate.empty()) {
            Caso btCasos[] = {
                { "hit_random", rDate     },
                { "miss       ", mDate    },
                { "hit_first  ", firstDate },
                { "hit_last   ", lastDate  }
            };
            for (auto& caso : btCasos) {
                double total = 0, minT = 1e18, maxT = 0;
                for (int rep = 0; rep < _M; ++rep) {
                    auto t0 = Clock::now();
                    for (int n = 0; n < _N; ++n) _branch.rawBTree().search(caso.key);
                    auto t1 = Clock::now();
                    double e = Us(t1 - t0).count();
                    total += e;
                    if (e < minT) minT = e;
                    if (e > maxT) maxT = e;
                }
                auto& r = _results[_resultCount++];
                r.operation = "busqueda";
                r.structure = "BTree";
                r.caseType  = caso.label;
                r.avgTimeUs = total / (_M * _N);
                r.minTimeUs = minT  / _N;
                r.maxTimeUs = maxT  / _N;
                r.N = _N; r.M = _M;
            }
        }
    }

    // ── Por categoría: BPlus ──────────────────────────────────────────────────
    {
        std::string rCat, firstCat, lastCat;
        const std::string mCat = "ZZZZZ_NO_CATEGORIA";
        const ListNode* nd = _branch.rawList().getHead();
        if (nd) {
            rCat = firstCat = lastCat = nd->data.category;
            while (nd) {
                if (nd->data.category < firstCat) firstCat = nd->data.category;
                if (nd->data.category > lastCat)  lastCat  = nd->data.category;
                nd = nd->next;
            }
        }
        if (!rCat.empty()) {
            Caso bpCasos[] = {
                { "hit_random", rCat     },
                { "miss       ", mCat    },
                { "hit_first  ", firstCat },
                { "hit_last   ", lastCat  }
            };
            for (auto& caso : bpCasos) {
                double total = 0, minT = 1e18, maxT = 0;
                for (int rep = 0; rep < _M; ++rep) {
                    auto t0 = Clock::now();
                    for (int n = 0; n < _N; ++n) {
                        Product* bpRes[64]; int bpCnt = 0;
                        _branch.rawBPlus().searchCategory(caso.key, bpRes, bpCnt, 64);
                    }
                    auto t1 = Clock::now();
                    double e = Us(t1 - t0).count();
                    total += e;
                    if (e < minT) minT = e;
                    if (e > maxT) maxT = e;
                }
                auto& r = _results[_resultCount++];
                r.operation = "busqueda";
                r.structure = "BPlus";
                r.caseType  = caso.label;
                r.avgTimeUs = total / (_M * _N);
                r.minTimeUs = minT  / _N;
                r.maxTimeUs = maxT  / _N;
                r.N = _N; r.M = _M;
            }
        }
    }
}

// ── Insert ────────────────────────────────────────────────────────────────────

void BranchBenchmark::runInsertBenchmark() {
    Product bp = benchProduct();

    struct Est { const char* name; };
    Est ests[] = {
        { "LinkedList" }, { "SortedList" }, { "AVL" },
        { "BTree"      }, { "BPlus"      }, { "HashTable" }
    };

    for (auto& est : ests) {
        std::string name(est.name);
        double total = 0, minT = 1e18, maxT = 0;

        for (int rep = 0; rep < _M; ++rep) {
            double elapsed = 0;
            for (int n = 0; n < _N; ++n) {
                std::string varBC = bp.barcode + "_" + std::to_string(rep * _N + n);
                Product var = bp;
                var.barcode = varBC;
                var.name    = bp.name + std::to_string(rep * _N + n);

                auto t0 = Clock::now();
                if      (name == "LinkedList") _branch.rawList().insertFront(var);
                else if (name == "SortedList") _branch.rawSortedList().insertSorted(var);
                else if (name == "AVL")        _branch.rawAVL().insert(var);
                else if (name == "BTree")      _branch.rawBTree().insert(var);
                else if (name == "BPlus")      _branch.rawBPlus().insert(var);
                else if (name == "HashTable")  _branch.rawHash().insert(var);
                auto t1 = Clock::now();
                elapsed += Us(t1 - t0).count();

                // Clean up immediately
                if      (name == "LinkedList") _branch.rawList().remove(varBC);
                else if (name == "SortedList") _branch.rawSortedList().remove(var.name);
                else if (name == "AVL")        _branch.rawAVL().remove(var.name);
                else if (name == "BTree")      _branch.rawBTree().remove(var.expiry_date);
                else if (name == "BPlus")      _branch.rawBPlus().remove(var.category);
                else if (name == "HashTable")  _branch.rawHash().remove(varBC);
            }
            total += elapsed;
            if (elapsed < minT) minT = elapsed;
            if (elapsed > maxT) maxT = elapsed;
        }

        auto& r = _results[_resultCount++];
        r.operation = "insercion";
        r.structure = est.name;
        r.caseType  = "general  ";
        r.avgTimeUs = total / (_M * _N);
        r.minTimeUs = minT  / _N;
        r.maxTimeUs = maxT  / _N;
        r.N = _N; r.M = _M;
    }
}

// ── Remove ────────────────────────────────────────────────────────────────────

void BranchBenchmark::runRemoveBenchmark() {
    const ListNode* head = _branch.rawList().getHead();
    if (!head) return;
    Product subj = head->data;

    struct Est { const char* name; };
    Est ests[] = {
        { "LinkedList" }, { "SortedList" }, { "AVL" },
        { "BTree"      }, { "BPlus"      }, { "HashTable" }
    };

    for (auto& est : ests) {
        std::string name(est.name);
        double total = 0, minT = 1e18, maxT = 0;

        for (int rep = 0; rep < _M; ++rep) {
            double elapsed = 0;
            for (int n = 0; n < _N; ++n) {
                auto t0 = Clock::now();
                if      (name == "LinkedList") _branch.rawList().remove(subj.barcode);
                else if (name == "SortedList") _branch.rawSortedList().remove(subj.name);
                else if (name == "AVL")        _branch.rawAVL().remove(subj.name);
                else if (name == "BTree")      _branch.rawBTree().remove(subj.expiry_date);
                else if (name == "BPlus")      _branch.rawBPlus().remove(subj.category);
                else if (name == "HashTable")  _branch.rawHash().remove(subj.barcode);
                auto t1 = Clock::now();
                elapsed += Us(t1 - t0).count();

                // Reinsert for next iteration
                if      (name == "LinkedList") _branch.rawList().insertFront(subj);
                else if (name == "SortedList") _branch.rawSortedList().insertSorted(subj);
                else if (name == "AVL")        _branch.rawAVL().insert(subj);
                else if (name == "BTree")      _branch.rawBTree().insert(subj);
                else if (name == "BPlus")      _branch.rawBPlus().insert(subj);
                else if (name == "HashTable")  _branch.rawHash().insert(subj);
            }
            total += elapsed;
            if (elapsed < minT) minT = elapsed;
            if (elapsed > maxT) maxT = elapsed;
        }

        auto& r = _results[_resultCount++];
        r.operation = "eliminacion";
        r.structure = est.name;
        r.caseType  = "general    ";
        r.avgTimeUs = total / (_M * _N);
        r.minTimeUs = minT  / _N;
        r.maxTimeUs = maxT  / _N;
        r.N = _N; r.M = _M;
    }
}
