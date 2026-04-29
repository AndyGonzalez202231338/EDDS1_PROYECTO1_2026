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
  private api    = inject(ApiService);
  private branchSvc = inject(BranchService);

  branches = signal<Branch[]>([]);
  selectedBranchId = signal<number | null>(null);

  results = signal<BenchmarkResult[]>([]);
  loading = signal(false);
  error   = signal('');

  structures = computed(() => [...new Set(this.results().map((r) => r.structure))]);
  operations = computed(() => [...new Set(this.results().map((r) => r.operation))]);

  get selectedBranchName(): string {
    const id = this.selectedBranchId();
    if (id === null) return '';
    const b = this.branches().find(x => x.id === id);
    return b ? b.nombre : `Sucursal ${id}`;
  }

  ngOnInit(): void {
    this.branchSvc.getBranches().subscribe({
      next: (bs) => this.branches.set(bs)
    });
  }

  onBranchChange(event: Event): void {
    const value = (event.target as HTMLSelectElement).value;
    this.selectedBranchId.set(value === '' ? null : Number(value));
  }

  run() {
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
        const msg = e?.error?.error || 'Error al ejecutar el benchmark. Asegúrate de que la sucursal tenga productos.';
        this.error.set(msg);
        this.loading.set(false);
      },
    });
  }

  getResult(op: string, struct: string, caseType: string): BenchmarkResult | undefined {
    return this.results().find((r) => r.operation === op && r.structure === struct && r.caseType === caseType);
  }

  formatTime(us: number): string {
    if (us >= 1000) return (us / 1000).toFixed(2) + ' ms';
    return us.toFixed(1) + ' µs';
  }

  speedClass(us: number): string {
    if (us < 10)  return 'text-success fw-bold';
    if (us < 100) return 'text-warning';
    return 'text-danger';
  }

  structBadgeClass(struct: string): string {
    switch (struct) {
      case 'LinkedList':  return 'bg-danger';
      case 'SortedList':  return 'bg-warning text-dark';
      case 'AVL':         return 'bg-success';
      case 'BTree':       return 'bg-primary';
      case 'BPlus':       return 'bg-info text-dark';
      case 'HashTable':   return 'bg-secondary';
      default:            return 'bg-dark';
    }
  }
}
