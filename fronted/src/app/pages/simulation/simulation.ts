import {
  Component, OnInit, OnDestroy, ViewChild,
  ElementRef, ChangeDetectorRef, AfterViewChecked
} from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormBuilder, FormGroup, ReactiveFormsModule, Validators } from '@angular/forms';
import { DomSanitizer, SafeHtml } from '@angular/platform-browser';

import { BranchService } from '../../services/branch.service';
import { ApiService } from '../../services/api.service';

import { Branch } from '../../models/branch.model';
import { Product } from '../../models/product.model';

interface SimSnapshot {
  currentTick: number;
  branches: BranchSimSnapshot[];
  recentCompleted: CompletedTransfer[];
}

interface BranchSimSnapshot {
  branchId: number;
  branchName: string;
  colaIngreso: QueueEntry[];
  colaTraspaso: QueueEntry[];
  colaSalida: QueueEntry[];
}

interface QueueEntry {
  transferId: string;
  barcode: string;
  productName: string;
  waitTicks: number;
  etaTicks: number;
  stock: number;
}

interface CompletedTransfer {
  transferId: string;
  barcode: string;
  completedTick: number;
  originId: number;
  destId: number;
  ok: boolean;
  error: string;
}

interface BatchItem {
  index: number;
  barcode: string;
  originId: number;
  originName: string;
  destId: number;
  destName: string;
  quantity: number;
  byTime: boolean;
  label: string;
}

@Component({
  selector: 'app-simulation',
  standalone: true,
  imports: [CommonModule, ReactiveFormsModule],
  templateUrl: './simulation.html',
  styleUrl: './simulation.css'
})
export class SimulationComponent implements OnInit, OnDestroy, AfterViewChecked {
  @ViewChild('graphContainer') graphContainerRef?: ElementRef;

  branches: Branch[] = [];
  branchProducts: Product[] = [];
  selectedProduct: Product | null = null;

  form: FormGroup;
  loadingProducts = false;
  submitting = false;
  submitError = '';
  submitSuccess = '';

  routePreview: { nodes: string[]; distance: number } | null = null;
  loadingRoute = false;

  pendingBatch: BatchItem[] = [];
  executingBatch = false;
  batchResults: any[] = [];

  snapshot: SimSnapshot | null = null;
  connected = false;

  graphSvg: SafeHtml | null = null;
  loadingGraph = false;
  private svgNeedsHighlight = false;

  transferColors = new Map<string, string>();
  colors = ['#FF6B6B', '#4ECDC4', '#45B7D1', '#FFA07A', '#98D8C8', '#F7DC6F', '#BB8FCE', '#85C1E2'];
  colorIndex = 0;

  private pollTimer?: ReturnType<typeof setInterval>;
  private visibilityHandler = () => {
    if (document.visibilityState === 'hidden') this.stopPolling();
    else this.startPolling();
  };

  constructor(
    private fb: FormBuilder,
    private branchService: BranchService,
    private apiService: ApiService,
    private sanitizer: DomSanitizer,
    private cdr: ChangeDetectorRef
  ) {
    this.form = this.fb.group({
      originId: [null, [Validators.required, Validators.min(1)]],
      destId: [null, [Validators.required, Validators.min(1)]],
      barcode: ['', Validators.required],
      quantity: [1, [Validators.required, Validators.min(1)]],
      criteria: ['time'],
      label: ['']
    });
  }

  ngOnInit(): void {
    this.branchService.getBranches().subscribe({
      next: (b: Branch[]) => {
        this.branches = b;
        this.cdr.markForCheck();
      }
    });
    this.loadGraph();
    this.refreshBatch();
    this.startPolling();
    document.addEventListener('visibilitychange', this.visibilityHandler);
  }

  ngOnDestroy(): void {
    this.stopPolling();
    document.removeEventListener('visibilitychange', this.visibilityHandler);
  }

  ngAfterViewChecked(): void {
    if (this.svgNeedsHighlight) {
      this.svgNeedsHighlight = false;
      this.highlightActiveBranches();
    }
  }

  // Polling
  private startPolling(): void {
    if (this.pollTimer) return;
    this.pollTimer = setInterval(() => this.fetchSnapshot(), 1000);
    this.fetchSnapshot();
  }

  private stopPolling(): void {
    if (this.pollTimer) {
      clearInterval(this.pollTimer);
      this.pollTimer = undefined;
    }
  }

  private fetchSnapshot(): void {
    this.apiService.runBranchBenchmark(1).subscribe({
      next: () => {},
      error: () => {}
    });

    fetch('http://localhost:8080/api/sim/state')
      .then(r => r.json())
      .then((snap: SimSnapshot) => {
        this.snapshot = snap;
        this.connected = true;
        this.svgNeedsHighlight = true;
        this.cdr.markForCheck();
      })
      .catch(() => {
        this.connected = false;
        this.cdr.markForCheck();
      });
  }

  // Form interactions
  onOriginChange(): void {
    const originId = Number(this.form.get('originId')?.value);
    this.branchProducts = [];
    this.selectedProduct = null;
    this.form.patchValue({ barcode: '', quantity: 1 });
    this.routePreview = null;
    if (!originId || originId <= 0) return;

    this.loadingProducts = true;
    this.branchService.getBranchProducts(originId).subscribe({
      next: (products: Product[]) => {
        this.branchProducts = products.filter(p => p.stock > 0);
        this.loadingProducts = false;
        this.cdr.markForCheck();
      },
      error: () => {
        this.loadingProducts = false;
        this.cdr.markForCheck();
      }
    });
  }

  onProductSelect(barcode: string): void {
    this.selectedProduct = this.branchProducts.find(p => p.barcode === barcode) ?? null;
    if (this.selectedProduct) {
      this.form.patchValue({ barcode, quantity: 1 });
      const quantityControl = this.form.get('quantity');
      if (quantityControl) {
        quantityControl.setValidators([
          Validators.required,
          Validators.min(1),
          Validators.max(this.selectedProduct.stock)
        ]);
        quantityControl.updateValueAndValidity();
      }
    }
  }

  onDestChange(): void {
    // noop
  }

  onCriteriaChange(): void {
    // noop
  }

  // Ver ruta
  viewRoute(): void {
    const originId = Number(this.form.get('originId')?.value);
    const destId = Number(this.form.get('destId')?.value);
    const criteria = this.form.get('criteria')?.value || 'time';

    this.routePreview = null;
    if (!originId || !destId || originId === destId) {
      this.submitError = 'Selecciona sucursales diferentes';
      return;
    }

    this.loadingRoute = true;
    fetch(`http://localhost:8080/api/graph/path?from=${originId}&to=${destId}&criteria=${criteria}`)
      .then(r => r.json())
      .then((res: any) => {
        if (res.path) {
          this.routePreview = {
            nodes: res.path.map((id: number) => this.branchName(id)),
            distance: res.distance || 0
          };
        }
        this.loadingRoute = false;
        this.cdr.markForCheck();
      })
      .catch(() => {
        this.loadingRoute = false;
        this.cdr.markForCheck();
      });
  }

  // Batch management
  async addToBatch(): Promise<void> {
    if (this.form.invalid) {
      this.form.markAllAsTouched();
      return;
    }

    const v = this.form.value;
    const originId = Number(v.originId);
    const destId = Number(v.destId);
    const quantity = Number(v.quantity);

    if (originId === destId) {
      this.submitError = 'Origen y destino deben ser diferentes';
      return;
    }

    this.submitting = true;
    this.submitError = '';
    this.submitSuccess = '';

    try {
      const res = await fetch('http://localhost:8080/api/sim/batch/add', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          barcode: v.barcode,
          originId,
          destId,
          quantity,
          byTime: v.criteria === 'time',
          label: v.label || '',
          criteria: v.criteria
        })
      });

      const data = await res.json();
      if (data.ok) {
        this.submitSuccess = `Agregado al batch (${data.pendingCount} pendientes)`;
        this.form.patchValue({ barcode: '', quantity: 1, label: '' });
        this.selectedProduct = null;
        this.branchProducts = [];
        this.routePreview = null;
        this.refreshBatch();
      } else {
        this.submitError = data.error || 'Error desconocido';
      }
    } catch (err) {
      this.submitError = 'Error al agregar al batch';
    }

    this.submitting = false;
    this.cdr.markForCheck();
  }

  async refreshBatch(): Promise<void> {
    try {
      const res = await fetch('http://localhost:8080/api/sim/batch');
      const data = await res.json();
      this.pendingBatch = data.pending || [];
      this.cdr.markForCheck();
    } catch (err) {
      console.error('Error al refrescar batch:', err);
    }
  }

  async clearBatch(): Promise<void> {
    if (!confirm('¿Limpiar todo el batch?')) return;

    try {
      await fetch('http://localhost:8080/api/sim/batch', { method: 'DELETE' });
      this.pendingBatch = [];
      this.batchResults = [];
      this.cdr.markForCheck();
    } catch (err) {
      console.error('Error al limpiar batch:', err);
    }
  }

  async executeBatch(): Promise<void> {
    if (this.pendingBatch.length === 0) {
      this.submitError = 'No hay transferencias pendientes';
      return;
    }

    this.executingBatch = true;
    this.submitError = '';
    this.submitSuccess = '';
    this.batchResults = [];

    try {
      const res = await fetch('http://localhost:8080/api/sim/batch/execute', {
        method: 'POST'
      });
      const data = await res.json();

      // Asignar colores
      (data.results || []).forEach((r: any) => {
        if (r.ok && r.transferId) {
          this.transferColors.set(r.transferId, this.colors[this.colorIndex % this.colors.length]);
          this.colorIndex++;
        }
      });

      this.batchResults = data.results || [];
      this.submitSuccess = `${data.executed || 0} transferencias ejecutadas`;
      this.pendingBatch = [];
    } catch (err) {
      this.submitError = 'Error al ejecutar batch';
    }

    this.executingBatch = false;
    this.cdr.markForCheck();
  }

  // Graph
  loadGraph(): void {
    this.loadingGraph = true;
    fetch('http://localhost:8080/api/graph/dot')
      .then(r => r.json())
      .then((data: any) => {
        let svgContent = data.svg || '';
        if (svgContent) {
          const processed = svgContent
            .replace(/(<svg[^>]*)\swidth="[^"]*pt"/, '$1 width="100%"')
            .replace(/(<svg[^>]*)\sheight="[^"]*pt"/, '$1 height="auto"');
          this.graphSvg = this.sanitizer.bypassSecurityTrustHtml(processed);
        } else {
          this.graphSvg = null;
        }
        this.svgNeedsHighlight = true;
        this.loadingGraph = false;
        this.cdr.markForCheck();
      })
      .catch(() => {
        this.loadingGraph = false;
        this.cdr.markForCheck();
      });
  }

  private highlightActiveBranches(): void {
    const container = this.graphContainerRef?.nativeElement as HTMLElement | undefined;
    if (!container || !this.snapshot) return;

    const activeIdSet = new Set(
      this.snapshot.branches
        .filter(b => this.totalItems(b) > 0)
        .map(b => String(b.branchId))
    );

    container.querySelectorAll<SVGGElement>('g.node').forEach(node => {
      const title = node.querySelector('title')?.textContent?.trim() ?? '';
      const shape = node.querySelector<SVGElement>('polygon, ellipse, rect');
      if (!shape) return;

      if (activeIdSet.has(title)) {
        shape.style.fill = '#ff6b6b';
        shape.style.stroke = '#dc2626';
        shape.style.strokeWidth = '3';
      }
    });
  }

  // Helpers
  branchName(id: number): string {
    return this.branches.find(b => b.id === id)?.nombre ?? `Sucursal ${id}`;
  }

  totalItems(b: BranchSimSnapshot): number {
    return b.colaIngreso.length + b.colaTraspaso.length + b.colaSalida.length;
  }

  get activeBranches(): BranchSimSnapshot[] {
    return (this.snapshot?.branches ?? []).filter(b => this.totalItems(b) > 0);
  }

  get completedSlice() {
    return (this.snapshot?.recentCompleted ?? []).slice().reverse().slice(0, 20);
  }

  isInvalid(field: string): boolean {
    const c = this.form.get(field);
    return !!(c?.invalid && c?.touched);
  }

  get maxQuantity(): number {
    return this.selectedProduct?.stock ?? 9999;
  }

  get currentTick(): number {
    return this.snapshot?.currentTick ?? 0;
  }

  getTransferColor(transferId: string): string {
    return this.transferColors.get(transferId) || '#3b82f6';
  }
}
