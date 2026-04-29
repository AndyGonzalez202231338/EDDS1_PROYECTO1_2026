import { Component, OnDestroy, OnInit, ChangeDetectorRef } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormBuilder, FormsModule, ReactiveFormsModule, Validators, FormGroup } from '@angular/forms';
import { RouterLink } from '@angular/router';
import { Subscription } from 'rxjs';

import { InventoryService } from '../../services/inventory.service';
import { StateService } from '../../services/state.service';
import { BranchSelectorComponent } from '../branch-selector/branch-selector';
import { Product } from '../../models/product.model';

@Component({
  selector: 'app-product',
  standalone: true,
  imports: [CommonModule, FormsModule, ReactiveFormsModule, RouterLink, BranchSelectorComponent],
  templateUrl: './product.html'
})
export class ProductComponent implements OnInit, OnDestroy {
  branchId: number | null = null;
  tab: 'list' | 'add' | 'search' | 'delete' = 'list';
  private sub?: Subscription;

  message = '';
  error = '';
  lastTimeUs: number | null = null;
  foundProduct: Product | null = null;
  foundTimeUs: number | null = null;

  // Listado
  products: Product[] = [];
  loadingProducts = false;
  filterText = '';

  addForm!: FormGroup;
  searchForm!: FormGroup;
  deleteForm!: FormGroup;

  constructor(
    private fb: FormBuilder,
    private inventory: InventoryService,
    private state: StateService,
    private cdr: ChangeDetectorRef
  ) {
    this.addForm = this.fb.group({
      name:        ['', Validators.required],
      barcode:     ['', Validators.required],
      category:    ['', Validators.required],
      expiry_date: ['', Validators.required],
      brand:       ['', Validators.required],
      price:  [0, [Validators.required, Validators.min(0.01)]],
      stock:  [0, [Validators.required, Validators.min(0)]]
    });
    this.searchForm = this.fb.group({ barcode: ['', Validators.required] });
    this.deleteForm = this.fb.group({ barcode: ['', Validators.required] });
  }

  ngOnInit(): void {
    this.sub = this.state.branchId$.subscribe((id) => {
      this.branchId = id;
      if (this.tab === 'list' && id !== null) {
        this.loadProducts();
      } else {
        this.products = [];
      }
      this.cdr.markForCheck();
    });
  }

  ngOnDestroy(): void {
    this.sub?.unsubscribe();
  }

  switchTab(t: 'list' | 'add' | 'search' | 'delete'): void {
    this.tab = t;
    this.resetMessages();
    this.foundProduct = null;
    this.foundTimeUs = null;
    if (t === 'list' && this.branchId !== null) {
      this.loadProducts();
    }
  }

  // ── Listado ──────────────────────────────────────────────────────────────

  loadProducts(): void {
    if (!this.branchId) return;
    this.loadingProducts = true;
    this.error = '';
    this.inventory.listProducts(this.branchId).subscribe({
      next: (data) => {
        this.products = data;
        this.loadingProducts = false;
        this.cdr.markForCheck();
      },
      error: (err) => {
        this.error = err?.error?.error || 'Error al cargar productos';
        this.loadingProducts = false;
        this.cdr.markForCheck();
      }
    });
  }

  get filteredProducts(): Product[] {
    const q = this.filterText.trim().toLowerCase();
    if (!q) return this.products;
    return this.products.filter(p =>
      p.name.toLowerCase().includes(q) ||
      p.barcode.toLowerCase().includes(q) ||
      p.category.toLowerCase().includes(q) ||
      p.brand.toLowerCase().includes(q)
    );
  }

  // ── Agregar ───────────────────────────────────────────────────────────────

  addProduct(): void {
    this.resetMessages();
    if (!this.branchId) return;
    if (this.addForm.invalid) { this.addForm.markAllAsTouched(); return; }

    this.inventory.insertProduct(this.branchId, this.addForm.value as Product).subscribe({
      next: (res) => {
        this.lastTimeUs = res.timeUs ?? null;
        this.message = 'Producto agregado correctamente';
        this.addForm.reset({ price: 0, stock: 0 });
        this.cdr.markForCheck();
      },
      error: (err) => {
        this.error = err?.error?.error || 'Error al agregar';
        this.cdr.markForCheck();
      }
    });
  }

  // ── Buscar ────────────────────────────────────────────────────────────────

  findProduct(): void {
    this.resetMessages();
    this.foundProduct = null;
    if (!this.branchId) return;
    const barcode = this.searchForm.value.barcode;
    if (!barcode) return;

    this.inventory.searchByBarcode(this.branchId, barcode).subscribe({
      next: (p) => {
        this.foundTimeUs = p.timeUs ?? null;
        this.foundProduct = p;
        this.cdr.markForCheck();
      },
      error: (err) => {
        this.error = err?.error?.error || 'Producto no encontrado';
        this.cdr.markForCheck();
      }
    });
  }

  // ── Eliminar ──────────────────────────────────────────────────────────────

  deleteProduct(): void {
    this.resetMessages();
    if (!this.branchId) return;
    const barcode = this.deleteForm.value.barcode;
    if (!barcode) return;

    this.inventory.removeProduct(this.branchId, barcode).subscribe({
      next: (res) => {
        this.lastTimeUs = res.timeUs ?? null;
        this.message = 'Producto eliminado correctamente';
        this.deleteForm.reset();
        this.cdr.markForCheck();
        if (this.tab === 'list') this.loadProducts();
      },
      error: (err) => {
        this.error = err?.error?.error || 'Error al eliminar';
        this.cdr.markForCheck();
      }
    });
  }

  formatTime(us: number): string {
    if (us >= 1000) return (us / 1000).toFixed(2) + ' ms';
    return us.toFixed(1) + ' µs';
  }

  private resetMessages(): void {
    this.message = '';
    this.error = '';
    this.lastTimeUs = null;
  }
}
