import { Component, signal, computed, inject, OnInit, ChangeDetectorRef } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { ApiService, DotFile, Product } from '../../services/api.service';

type SearchMode = 'name' | 'barcode' | 'category' | 'daterange';

@Component({
  selector: 'app-catalog',
  imports: [FormsModule],
  templateUrl: './catalog.html',
  styleUrl: './catalog.css',
})
export class Catalog implements OnInit {
  private api = inject(ApiService);
  private cdr = inject(ChangeDetectorRef);

  protected Math = Math;

  // Productos 
  products = signal<Product[]>([]);
  loading = signal(false);
  error = signal('');
  success = signal('');
  apiOnline = signal<boolean | null>(null);

  // Búsqueda 
  searchMode: SearchMode = 'name';
  searchQuery = '';
  dateFrom = '';
  dateTo = '';

  // Paginación 
  readonly pageSize = 10;
  currentPage = signal(1);
  totalPages = computed(() => Math.max(1, Math.ceil(this.products().length / this.pageSize)));
  pagedProducts = computed(() => {
    const start = (this.currentPage() - 1) * this.pageSize;
    return this.products().slice(start, start + this.pageSize);
  });
  pages = computed(() => Array.from({ length: this.totalPages() }, (_, i) => i + 1));

  // Modales 
  showAddModal = signal(false);
  newProduct: Product = { name: '', barcode: '', category: '', expiry_date: '', brand: '', price: 0, stock: 0, status: 'AVAILABLE', branchId: 0 };

  showLoadCsvModal = signal(false);
  selectedFileName = signal('');
  csvContent = '';

  // DOT / Árboles
  dotLabel = 'snapshot';
  dotFiles = signal<DotFile[]>([]);
  dotLabelResult = signal('');
  pngFiles = computed(() => this.dotFiles().filter(f => f.name.endsWith('.png')));
  dotForPng(pngName: string): string { return pngName.replace('.png', '.dot'); }
  treeName(name: string): string {
    if (name.includes('_AVL.'))   return 'Árbol AVL';
    if (name.includes('_BTree.')) return 'Árbol B';
    if (name.includes('_BPlus.')) return 'Árbol B+';
    return name;
  }

  // Consola de errores 
  errorLog = signal('');
  showErrorLog = signal(false);
  errorLogLoading = signal(false);

  ngOnInit() {
    this.checkApi();
    this.loadAll();
  }

  checkApi() {
    this.api.ping().subscribe({
      next: () => this.apiOnline.set(true),
      error: () => this.apiOnline.set(false),
    });
  }

  loadAll() {
    this.loading.set(true);
    this.error.set('');
    this.api.getProducts().subscribe({
      next: (products) => {
        this.products.set(products);
        this.currentPage.set(1);
        this.loading.set(false);
      },
      error: () => {
        this.error.set('No se pudo conectar al servidor C++ (puerto 8080). Asegúrate de que el servidor esté corriendo.');
        this.loading.set(false);
      },
    });
  }

  search() {
    if (this.searchMode === 'daterange') {
      if (!this.dateFrom || !this.dateTo) { this.error.set('Ingresa ambas fechas para la búsqueda por rango.'); return; }
      this.loading.set(true);
      this.error.set('');
      this.api.searchByDateRange(this.dateFrom, this.dateTo).subscribe({
        next: (ps) => { this.products.set(ps); this.currentPage.set(1); this.loading.set(false); },
        error: () => { this.error.set('Sin resultados en ese rango de fechas.'); this.products.set([]); this.loading.set(false); },
      });
      return;
    }
    if (!this.searchQuery.trim()) { this.loadAll(); return; }
    this.loading.set(true);
    this.error.set('');
    switch (this.searchMode) {
      case 'name':
        this.api.searchByName(this.searchQuery).subscribe({
          next: (p) => { this.products.set([p]); this.currentPage.set(1); this.loading.set(false); },
          error: () => { this.error.set('Producto no encontrado.'); this.products.set([]); this.loading.set(false); },
        });
        break;
      case 'barcode':
        this.api.searchByBarcode(this.searchQuery).subscribe({
          next: (p) => { this.products.set([p]); this.currentPage.set(1); this.loading.set(false); },
          error: () => { this.error.set('Producto no encontrado.'); this.products.set([]); this.loading.set(false); },
        });
        break;
      case 'category':
        this.api.searchByCategory(this.searchQuery).subscribe({
          next: (ps) => { this.products.set(ps); this.currentPage.set(1); this.loading.set(false); },
          error: () => { this.error.set('Sin resultados para esa categoría.'); this.products.set([]); this.loading.set(false); },
        });
        break;
    }
  }

  clearSearch() {
    this.searchQuery = '';
    this.dateFrom = '';
    this.dateTo = '';
    this.searchMode = 'name';
    this.loadAll();
  }

  deleteProduct(barcode: string) {
    if (!confirm(`¿Eliminar el producto con código ${barcode}?`)) return;
    this.api.deleteProduct(barcode).subscribe({
      next: () => { this.success.set('Producto eliminado correctamente.'); this.loadAll(); },
      error: () => this.error.set('Error al eliminar el producto.'),
    });
  }

  openAddModal() {
    this.newProduct = { name: '', barcode: '', category: '', expiry_date: '', brand: '', price: 0, stock: 0, status: 'AVAILABLE', branchId: 0 };
    this.error.set('');
    this.showAddModal.set(true);
  }

  submitAddProduct() {
    this.api.addProduct(this.newProduct).subscribe({
      next: () => {
        this.showAddModal.set(false);
        this.success.set(`Producto "${this.newProduct.name}" agregado exitosamente.`);
        this.loadAll();
      },
      error: () => this.error.set('Error al agregar el producto. Verifica que el código de barras no esté duplicado.'),
    });
  }

  openLoadCsvModal() {
    this.selectedFileName.set('');
    this.csvContent = '';
    this.showLoadCsvModal.set(true);
  }

  onFileSelected(event: Event) {
    const file = (event.target as HTMLInputElement).files?.[0];
    if (!file) return;
    this.selectedFileName.set(file.name);
    const reader = new FileReader();
    reader.onload = () => { this.csvContent = reader.result as string; };
    reader.readAsText(file);
  }

  submitLoadCSV() {
    if (!this.csvContent) { 
      this.error.set('Selecciona un archivo CSV primero.'); 
      return; 
    }
    this.api.uploadCSV(this.csvContent).subscribe({
      next: (r) => {
        this.showLoadCsvModal.set(false);
        this.success.set(`CSV cargado: ${r.loaded} productos insertados, ${r.errors} errores.`);
        this.csvContent = '';
        this.selectedFileName.set('');
        this.loadAll();
        if (r.errors > 0) {
          this.loadErrors();
          this.cdr.detectChanges(); // ← Aquí también
        }
      },
      error: () => this.error.set('Error al cargar el CSV.'),
    });
  }

  generateDot() {
    this.api.generateDot(this.dotLabel).subscribe({
      next: (r) => {
        this.dotFiles.set(r.files);
        this.dotLabelResult.set(r.label);
        const pngs = r.files.filter(f => f.name.endsWith('.png')).length;
        this.success.set(
          pngs > 0
            ? `Árboles generados: ${pngs} imágenes disponibles para descargar.`
            : `Archivos DOT generados. Instala Graphviz para obtener imágenes PNG.`
        );
      },
      error: () => this.error.set('Error al generar los árboles DOT.'),
    });
  }

  loadErrors() {
    this.errorLogLoading.set(true);
    this.api.getErrors().subscribe({
      next: (r) => {
        this.errorLog.set(r.log?.trim() || 'Sin errores registrados.');
        this.errorLogLoading.set(false);
        this.cdr.detectChanges(); // ← Cambiado de markForCheck()
      },
      error: () => {
        this.errorLog.set('No se pudo leer el log de errores.');
        this.errorLogLoading.set(false);
        this.cdr.detectChanges(); // ← Cambiado de markForCheck()
      },
    });
  }

  toggleErrorLog() {
    this.showErrorLog.update(v => !v);
    if (this.showErrorLog() && !this.errorLog()) {
      this.loadErrors();
    }
    this.cdr.detectChanges(); // ← Añade aquí también
  }

  clearErrorLog() {
    this.api.clearErrors().subscribe({
      next: () => {
        this.errorLog.set('Log limpiado.');
        this.success.set('Log de errores limpiado.');
        this.cdr.detectChanges(); // ← Cambiado de markForCheck()
      },
      error: () => {
        this.error.set('Error al limpiar el log.');
        this.cdr.detectChanges();
      },
    });
  }

  setPage(page: number) {
    if (page >= 1 && page <= this.totalPages()) this.currentPage.set(page);
  }

  isExpiringSoon(date: string): boolean {
    const expiry = new Date(date);
    const diff = expiry.getTime() - Date.now();
    return diff > 0 && diff < 30 * 24 * 60 * 60 * 1000;
  }

  clearMessages() {
    this.error.set('');
    this.success.set('');
  }
}
