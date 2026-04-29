import { Component, OnInit, OnDestroy, ChangeDetectorRef } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormBuilder, FormGroup, ReactiveFormsModule, Validators } from '@angular/forms';
import { interval, Subscription } from 'rxjs';
import { switchMap, takeWhile } from 'rxjs/operators';
import { BranchService } from '../../../services/branch.service';
import { TransferService } from '../../../services/transfer.service';
import { Branch } from '../../../models/branch.model';
import { TransferResponse, TransferProgress } from '../../../models/transfer.model';

export interface QueueEvent {
  branchName: string;
  branchId: number;
  events: string[];
  isDestination: boolean;
}

@Component({
  selector: 'app-transfer',
  standalone: true,
  imports: [CommonModule, ReactiveFormsModule],
  templateUrl: './transfer-form.html',
  styleUrl: './transfer-form.css'
})
export class TransferFormComponent implements OnInit, OnDestroy {
  branches: Branch[] = [];
  form: FormGroup;
  loading = false;

  // fast mode
  result: TransferResponse | null = null;
  queueFlow: QueueEvent[] = [];

  // realtime mode
  progress: TransferProgress | null = null;
  private pollSub?: Subscription;

  constructor(
    private fb: FormBuilder,
    private branchService: BranchService,
    private transferService: TransferService,
    private cdr: ChangeDetectorRef
  ) {
    this.form = this.fb.group({
      barcode:  ['', Validators.required],
      originId: [null, [Validators.required, Validators.min(1)]],
      destId:   [null, [Validators.required, Validators.min(1)]],
      type:     ['time', Validators.required],
      mode:     ['fast', Validators.required]
    });
  }

  ngOnInit(): void {
    this.branchService.getBranches().subscribe({
      next: (b) => { this.branches = b; this.cdr.markForCheck(); },
      error: () => {}
    });
  }

  ngOnDestroy(): void {
    this.pollSub?.unsubscribe();
  }

  get isRealtime(): boolean {
    return this.form.get('mode')?.value === 'realtime';
  }

  submit(): void {
    if (this.form.invalid) { this.form.markAllAsTouched(); return; }

    this.loading = true;
    this.result  = null;
    this.progress = null;
    this.queueFlow = [];
    this.pollSub?.unsubscribe();

    const v = this.form.value;

    this.transferService.transfer({
      barcode:  v.barcode,
      originId: Number(v.originId),
      destId:   Number(v.destId),
      type:     v.type,
      mode:     v.mode
    }).subscribe({
      next: (res) => {
        if (v.mode === 'realtime' && res.transferId) {
          this.loading = false;
          this.startPolling(res.transferId);
        } else {
          this.loading = false;
          this.result  = res;
          if (res.ok && res.steps && res.path) {
            this.buildQueueFlow(res.path, res.steps);
          }
        }
        this.cdr.markForCheck();
      },
      error: (err) => {
        this.loading = false;
        this.result  = { ok: false, error: err?.error?.error || 'Error de conexión' };
        this.cdr.markForCheck();
      }
    });
  }

  private startPolling(transferId: string): void {
    this.pollSub = interval(1000).pipe(
      switchMap(() => this.transferService.getStatus(transferId)),
      takeWhile((p) => !p.completed, true)
    ).subscribe({
      next: (p) => {
        this.progress = p;
        this.cdr.markForCheck();
      },
      error: () => { this.pollSub?.unsubscribe(); }
    });
  }

  branchName(id: number): string {
    const b = this.branches.find(x => x.id === id);
    return b ? b.nombre : `Sucursal ${id}`;
  }

  // ── Fast mode helpers ─────────────────────────────────────────────────────

  private buildQueueFlow(path: number[], steps: string[]): void {
    this.queueFlow = [];
    let current: QueueEvent | null = null;

    for (const step of steps) {
      if (step.startsWith('Ingreso en ')) {
        if (current) this.queueFlow.push(current);
        const m = step.match(/^Ingreso en (.+?) \(/);
        current = { branchName: m ? m[1] : step, branchId: 0, events: [step], isDestination: false };
      } else if (step.startsWith('Entrega en ')) {
        if (current) { current.events.push(step); current.isDestination = true; }
      } else {
        if (current) current.events.push(step);
      }
    }
    if (current) this.queueFlow.push(current);
    for (let i = 0; i < this.queueFlow.length && i < path.length; i++) {
      this.queueFlow[i].branchId = path[i];
    }
  }

  getPathString(): string {
    if (!this.result?.path) return '';
    return this.result.path.map(id => {
      const b = this.branches.find(x => x.id === id);
      return b ? `${b.nombre} (${id})` : String(id);
    }).join(' → ');
  }

  isInvalid(field: string): boolean {
    const c = this.form.get(field);
    return !!(c && c.invalid && c.touched);
  }

  stepClass(step: string): string {
    if (step.startsWith('Ingreso'))     return 'step-ingreso';
    if (step.startsWith('Preparacion')) return 'step-preparacion';
    if (step.startsWith('Salida'))      return 'step-salida';
    if (step.startsWith('Transito'))    return 'step-transito';
    if (step.startsWith('Entrega'))     return 'step-entrega';
    return '';
  }

  stepIcon(step: string): string {
    if (step.startsWith('Ingreso'))     return '📥';
    if (step.startsWith('Preparacion')) return '⚙️';
    if (step.startsWith('Salida'))      return '📤';
    if (step.startsWith('Transito'))    return '🚛';
    if (step.startsWith('Entrega'))     return '✅';
    return '•';
  }

  queueLabel(q: string): string {
    if (q === 'INGRESO')     return 'Ingreso';
    if (q === 'PREPARACION') return 'Preparación';
    if (q === 'SALIDA')      return 'Salida';
    return q;
  }

  queueBadgeClass(q: string): string {
    if (q === 'INGRESO')     return 'bg-primary';
    if (q === 'PREPARACION') return 'bg-warning text-dark';
    if (q === 'SALIDA')      return 'bg-success';
    return 'bg-secondary';
  }
}
