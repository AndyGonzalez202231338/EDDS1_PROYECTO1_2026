import { Component, signal, inject, computed, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { ApiService, BenchmarkResult } from '../../services/api.service';
import { BranchService } from '../../services/branch.service';
import { Branch } from '../../models/branch.model';

@Component({
  selector: 'app-benchmark',
  standalone: true,
  imports: [CommonModule, FormsModule],
  templateUrl: './benchmark.html',
  styleUrl: './benchmark.css',
})
export class Benchmark implements OnInit {
  private api       = inject(ApiService);
  private branchSvc = inject(BranchService);

  branches         = signal<Branch[]>([]);
  selectedBranchId = signal<number | null>(null);
  results          = signal<BenchmarkResult[]>([]);
  loading          = signal(false);
  error            = signal('');
  activeTab        = signal<'busqueda' | 'insercion' | 'eliminacion'>('busqueda');

  private readonly STRUCTURE_ORDER       = ['LinkedList', 'SortedList', 'HashTable', 'AVL', 'BTree', 'BPlus'];
  private readonly SEARCH_STRUCTURE_ORDER = ['LinkedList', 'HashTable', 'AVL', 'BTree', 'BPlus'];
  readonly SEARCH_CASES = ['hit_random', 'miss', 'hit_first', 'hit_last'];

  readonly caseLabels: Record<string, string> = {
    hit_random: 'Aleatorio',
    miss:       'Miss',
    hit_first:  'Mejor caso',
    hit_last:   'Peor caso',
    general:    'General',
  };

  readonly structureLabels: Record<string, string> = {
    LinkedList: 'LinkedList',
    SortedList: 'SortedList',
    HashTable:  'HashTable',
    AVL:        'AVL',
    BTree:      'B-Tree',
    BPlus:      'B+ Tree',
  };

  // Trim trailing spaces that the C++ benchmark adds to caseType strings
  normalizedResults = computed(() =>
    this.results().map(r => ({ ...r, caseType: r.caseType.trim() }))
  );

  hasResults = computed(() => this.results().length > 0);

  searchResults = computed(() =>
    this.normalizedResults().filter(r => r.operation === 'busqueda')
  );

  insertResults = computed(() =>
    this.normalizedResults().filter(r => r.operation === 'insercion')
  );

  removeResults = computed(() =>
    this.normalizedResults().filter(r => r.operation === 'eliminacion')
  );

  searchStructures = computed(() => {
    const found = new Set(this.searchResults().map(r => r.structure));
    return this.SEARCH_STRUCTURE_ORDER.filter(s => found.has(s));
  });

  insertStructures = computed(() => {
    const found = new Set(this.insertResults().map(r => r.structure));
    return this.STRUCTURE_ORDER.filter(s => found.has(s));
  });

  removeStructures = computed(() => {
    const found = new Set(this.removeResults().map(r => r.structure));
    return this.STRUCTURE_ORDER.filter(s => found.has(s));
  });

  // Max avg per column so cells can show a relative bar
  searchMaxByCase = computed(() => {
    const map: Record<string, number> = {};
    for (const c of this.SEARCH_CASES) {
      const vals = this.searchResults()
        .filter(r => r.caseType === c)
        .map(r => r.avgTimeUs);
      map[c] = vals.length ? Math.max(...vals) : 1;
    }
    return map;
  });

  insertMax = computed(() =>
    Math.max(1, ...this.insertResults().map(r => r.avgTimeUs))
  );

  removeMax = computed(() =>
    Math.max(1, ...this.removeResults().map(r => r.avgTimeUs))
  );

  get selectedBranchName(): string {
    const id = this.selectedBranchId();
    if (id === null) return '';
    const b = this.branches().find(x => x.id === id);
    return b ? b.nombre : `Sucursal ${id}`;
  }

  ngOnInit(): void {
    this.branchSvc.getBranches().subscribe({ next: (bs) => this.branches.set(bs) });
  }

  onBranchChange(event: Event): void {
    const value = (event.target as HTMLSelectElement).value;
    this.selectedBranchId.set(value === '' ? null : Number(value));
  }

  run(): void {
    const branchId = this.selectedBranchId();
    if (branchId === null) {
      this.error.set('Selecciona una sucursal para ejecutar el benchmark.');
      return;
    }
    this.loading.set(true);
    this.error.set('');
    this.api.runBranchBenchmark(branchId).subscribe({
      next:  (r) => { this.results.set(r); this.loading.set(false); },
      error: (e) => {
        this.error.set(e?.error?.error || 'Error al ejecutar el benchmark. Asegúrate de que la sucursal tenga productos.');
        this.loading.set(false);
      },
    });
  }

  getSearch(struct: string, caseType: string): BenchmarkResult | undefined {
    return this.searchResults().find(r => r.structure === struct && r.caseType === caseType);
  }

  getInsert(struct: string): BenchmarkResult | undefined {
    return this.insertResults().find(r => r.structure === struct);
  }

  getRemove(struct: string): BenchmarkResult | undefined {
    return this.removeResults().find(r => r.structure === struct);
  }

  formatTime(us: number | undefined): string {
    if (us === undefined) return '—';
    if (us >= 1000) return (us / 1000).toFixed(2) + ' ms';
    return us.toFixed(2) + ' µs';
  }

  speedClass(us: number | undefined): string {
    if (us === undefined) return 'text-muted';
    if (us < 10)  return 'text-success fw-bold';
    if (us < 100) return 'text-warning fw-semibold';
    return 'text-danger fw-semibold';
  }

  barWidth(us: number, max: number): string {
    return Math.max(4, Math.round((us / max) * 100)) + '%';
  }

  barColor(us: number): string {
    if (us < 10)  return '#198754';
    if (us < 100) return '#ffc107';
    return '#dc3545';
  }

  structBadgeClass(struct: string): string {
    switch (struct) {
      case 'LinkedList': return 'bg-danger';
      case 'SortedList': return 'bg-warning text-dark';
      case 'HashTable':  return 'bg-secondary';
      case 'AVL':        return 'bg-success';
      case 'BTree':      return 'bg-primary';
      case 'BPlus':      return 'bg-info text-dark';
      default:           return 'bg-dark';
    }
  }
}
