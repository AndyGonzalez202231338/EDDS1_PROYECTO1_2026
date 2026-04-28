import { Component, OnInit, ChangeDetectorRef } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ReactiveFormsModule, FormBuilder, FormGroup, Validators } from '@angular/forms';
import { BranchService } from '../../../services/branch.service';
import { StateService } from '../../../services/state.service';
import { Branch } from '../../../models/branch.model';

@Component({
  selector: 'app-branch-list',
  standalone: true,
  imports: [CommonModule, ReactiveFormsModule],
  templateUrl: './branch-list.html',
  styleUrl: './branch-list.css',
})
export class BranchListComponent implements OnInit {
  branches: Branch[] = [];
  selectedId: number | null = null;
  showForm = false;
  loading = false;
  saving = false;
  deletingId: number | null = null;

  message = '';
  messageType: 'success' | 'danger' = 'success';

  form: FormGroup;

  constructor(
    private branchService: BranchService,
    private state: StateService,
    private fb: FormBuilder,
    private cdr: ChangeDetectorRef
  ) {
    this.form = this.fb.group({
      id: [null, [Validators.required, Validators.min(1)]],
      nombre: ['', [Validators.required, Validators.minLength(2)]],
      ubicacion: ['', [Validators.required, Validators.minLength(2)]],
      tiempoIngreso: [5, [Validators.required, Validators.min(1)]],
      tiempoPreparacion: [10, [Validators.required, Validators.min(1)]],
      intervaloDespacho: [15, [Validators.required, Validators.min(1)]],
    });
  }

  ngOnInit(): void {
    this.selectedId = this.state.getBranchId();
    this.loadBranches();
  }

  loadBranches(): void {
    this.loading = true;
    this.branchService.getBranches().subscribe({
      next: (data) => {
        this.branches = data;
        this.loading = false;
        this.cdr.markForCheck();
      },
      error: () => {
        this.showMessage('No se pudieron cargar las sucursales', 'danger');
        this.loading = false;
        this.cdr.markForCheck();
      },
    });
  }

  selectBranch(id: number): void {
    this.selectedId = id;
    this.state.setBranchId(id);
  }

  toggleForm(): void {
    this.showForm = !this.showForm;
    if (!this.showForm) this.form.reset({ tiempoIngreso: 5, tiempoPreparacion: 10, intervaloDespacho: 15 });
    this.clearMessage();
  }

  onCreate(): void {
    if (this.form.invalid) {
      this.form.markAllAsTouched();
      return;
    }
    this.saving = true;
    this.branchService.createBranch(this.form.value as Branch).subscribe({
      next: () => {
        this.showMessage(`Sucursal "${this.form.value.nombre}" creada exitosamente`, 'success');
        this.form.reset({ tiempoIngreso: 5, tiempoPreparacion: 10, intervaloDespacho: 15 });
        this.showForm = false;
        this.saving = false;
        this.cdr.markForCheck();
        this.loadBranches();
      },
      error: (err) => {
        this.showMessage(err?.error?.error || 'Error al crear la sucursal', 'danger');
        this.saving = false;
        this.cdr.markForCheck();
      },
    });
  }

  onDelete(branch: Branch): void {
    if (!confirm(`¿Eliminar la sucursal "${branch.nombre}"?`)) return;
    this.deletingId = branch.id;
    this.branchService.deleteBranch(branch.id).subscribe({
      next: () => {
        this.showMessage(`Sucursal "${branch.nombre}" eliminada`, 'success');
        if (this.selectedId === branch.id) {
          this.selectedId = null;
          this.state.setBranchId(null);
        }
        this.deletingId = null;
        this.cdr.markForCheck();
        this.loadBranches();
      },
      error: (err) => {
        this.showMessage(err?.error?.error || 'Error al eliminar', 'danger');
        this.deletingId = null;
        this.cdr.markForCheck();
      },
    });
  }

  showMessage(msg: string, type: 'success' | 'danger'): void {
    this.message = msg;
    this.messageType = type;
  }

  clearMessage(): void {
    this.message = '';
  }

  isInvalid(field: string): boolean {
    const ctrl = this.form.get(field);
    return !!(ctrl && ctrl.invalid && ctrl.touched);
  }
}
