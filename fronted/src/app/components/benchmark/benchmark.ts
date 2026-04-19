import { Component, signal, inject, computed } from '@angular/core';
import { ApiService, BenchmarkResult } from '../../services/api.service';

@Component({
  selector: 'app-benchmark',
  imports: [],
  templateUrl: './benchmark.html',
  styleUrl: './benchmark.css',
})
export class Benchmark {
  private api = inject(ApiService);

  results = signal<BenchmarkResult[]>([]);
  loading = signal(false);
  error = signal('');

  structures = computed(() => [...new Set(this.results().map((r) => r.structure))]);
  operations = computed(() => [...new Set(this.results().map((r) => r.operation))]);

  run() {
    this.loading.set(true);
    this.error.set('');
    this.api.runBenchmark().subscribe({
      next: (r) => { this.results.set(r); this.loading.set(false); },
      error: () => { this.error.set('Error al ejecutar el benchmark. Asegúrate de que el servidor esté corriendo y tenga productos cargados.'); this.loading.set(false); },
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
    if (us < 10) return 'text-success fw-bold';
    if (us < 100) return 'text-warning';
    return 'text-danger';
  }
}
