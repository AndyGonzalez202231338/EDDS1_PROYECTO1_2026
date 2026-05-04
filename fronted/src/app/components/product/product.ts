import { Component, OnDestroy, OnInit, ChangeDetectorRef } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormBuilder, FormsModule, ReactiveFormsModule, Validators, FormGroup } from '@angular/forms';
import { RouterLink } from '@angular/router';
import { DomSanitizer, SafeHtml } from '@angular/platform-browser';
import { Subscription } from 'rxjs';

import { StateService } from '../../services/state.service';
import { BranchSelectorComponent } from '../branch-selector/branch-selector';
import { Product } from '../../models/product.model';
import { ApiService } from '../../services/api.service';

interface ProductCreatePayload {
  name: string;
  barcode: string;
  category: string;
  expiry_date: string;
  brand: string;
  price: number;
  stock: number;
}

interface ProductWithTime extends Product {
  timeUs?: number;
}

@Component({
  selector: 'app-product',
  standalone: true,
  imports: [CommonModule, FormsModule, ReactiveFormsModule, RouterLink, BranchSelectorComponent],
  templateUrl: './product.html',
  styles: [`
    .tree-svg-wrapper { min-height: 200px; max-height: 72vh; }
    .tree-svg-wrapper ::ng-deep svg { width: 100%; height: auto; display: block; }
  `]
})
export class ProductComponent implements OnInit, OnDestroy {
  branchId: number | null = null;
  tab: 'list' | 'add' | 'search' | 'delete' | 'trees' = 'list';
  searchType: 'barcode' | 'name' | 'category' | 'daterange' = 'barcode';
  private sub?: Subscription;

  message = '';
  error = '';
  lastTimeUs: number | null = null;
  foundProduct: ProductWithTime | null = null;
  foundTimeUs: number | null = null;
  foundProducts: ProductWithTime[] = [];

  // Listado
  products: Product[] = [];
  loadingProducts = false;
  filterText = '';
  sortAlpha = false;

  // Árboles
  treeSvg: { avl: string; btree: string; bplus: string; hash: string } | null = null;
  loadingTrees = false;
  activeTree: 'avl' | 'btree' | 'bplus' | 'hash' = 'avl';

  addForm!: FormGroup;
  searchForm!: FormGroup;
  searchNameForm!: FormGroup;
  searchCategoryForm!: FormGroup;
  searchDateRangeForm!: FormGroup;
  deleteForm!: FormGroup;

  constructor(
    private fb: FormBuilder,
    private api: ApiService,
    private state: StateService,
    private cdr: ChangeDetectorRef,
    private sanitizer: DomSanitizer
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
    this.searchNameForm = this.fb.group({ name: ['', Validators.required] });
    this.searchCategoryForm = this.fb.group({ category: ['', Validators.required] });
    this.searchDateRangeForm = this.fb.group({
      d1: ['', Validators.required],
      d2: ['', Validators.required]
    });
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

  switchTab(t: 'list' | 'add' | 'search' | 'delete' | 'trees'): void {
    this.tab = t;
    this.resetMessages();
    this.foundProduct = null;
    this.foundProducts = [];
    this.foundTimeUs = null;
    if (t === 'list' && this.branchId !== null) {
      this.loadProducts();
    }
  }

  switchSearchType(type: 'barcode' | 'name' | 'category' | 'daterange'): void {
    this.searchType = type;
    this.resetMessages();
    this.foundProduct = null;
    this.foundProducts = [];
    this.foundTimeUs = null;
  }

  loadProducts(): void {
    if (!this.branchId) return;
    this.loadingProducts = true;
    this.error = '';
    this.api.getBranchProducts(this.branchId).subscribe({
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
    let list = q
      ? this.products.filter(p =>
          p.name.toLowerCase().includes(q) ||
          p.barcode.toLowerCase().includes(q) ||
          p.category.toLowerCase().includes(q) ||
          p.brand.toLowerCase().includes(q)
        )
      : [...this.products];
    if (this.sortAlpha) list.sort((a, b) => a.name.localeCompare(b.name));
    return list;
  }

  addProduct(): void {
    this.resetMessages();
    if (!this.branchId) return;
    if (this.addForm.invalid) { this.addForm.markAllAsTouched(); return; }

    const payload: ProductCreatePayload = this.addForm.getRawValue();

    this.api.addProductToBranch(this.branchId, payload).subscribe({
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

  findProduct(): void {
    this.resetMessages();
    this.foundProduct = null;
    this.foundProducts = [];
    if (!this.branchId) return;
    const barcode = this.searchForm.value.barcode;
    if (!barcode) return;

    this.api.searchBranchProductByBarcode(this.branchId, barcode).subscribe({
      next: (p: ProductWithTime) => {
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

  findByName(): void {
    this.resetMessages();
    this.foundProduct = null;
    this.foundProducts = [];
    if (!this.branchId) return;
    const name = this.searchNameForm.value.name;
    if (!name) return;

    this.api.searchBranchProductByName(this.branchId, name).subscribe({
      next: (p: ProductWithTime) => {
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

  findByCategory(): void {
    this.resetMessages();
    this.foundProduct = null;
    this.foundProducts = [];
    if (!this.branchId) return;
    const category = this.searchCategoryForm.value.category;
    if (!category) return;

    this.api.searchBranchProductsByCategory(this.branchId, category).subscribe({
      next: (res) => {
        this.foundProducts = res.products as ProductWithTime[];
        this.foundTimeUs = res.timeUs ?? null;
        this.cdr.markForCheck();
      },
      error: (err) => {
        this.error = err?.error?.error || 'Error en búsqueda por categoría';
        this.cdr.markForCheck();
      }
    });
  }

  findByDateRange(): void {
    this.resetMessages();
    this.foundProduct = null;
    this.foundProducts = [];
    if (!this.branchId) return;
    const { d1, d2 } = this.searchDateRangeForm.value;
    if (!d1 || !d2) return;

    this.api.searchBranchProductsByDateRange(this.branchId, d1, d2).subscribe({
      next: (res) => {
        this.foundProducts = res.products as ProductWithTime[];
        this.foundTimeUs = res.timeUs ?? null;
        this.cdr.markForCheck();
      },
      error: (err) => {
        this.error = err?.error?.error || 'Error en búsqueda por rango de fechas';
        this.cdr.markForCheck();
      }
    });
  }

  deleteProduct(): void {
    this.resetMessages();
    if (!this.branchId) return;
    const barcode = this.deleteForm.value.barcode;
    if (!barcode) return;

    this.api.deleteBranchProduct(this.branchId, barcode).subscribe({
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

  safeSvg(svg: string): SafeHtml {
    return this.sanitizer.bypassSecurityTrustHtml(svg);
  }

  loadTrees(): void {
    this.resetMessages();
    this.treeSvg = null;
    if (!this.branchId) return;
    this.loadingTrees = true;

    this.api.getBranchTrees(this.branchId).subscribe({
      next: (res) => {
        this.treeSvg = res;
        this.loadingTrees = false;
        this.cdr.markForCheck();
      },
      error: (err) => {
        this.error = err?.error?.error || 'Error al generar los árboles';
        this.loadingTrees = false;
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
