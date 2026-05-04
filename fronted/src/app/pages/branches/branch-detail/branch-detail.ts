import { Component, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ActivatedRoute } from '@angular/router';
import { OperationHistoryComponent } from '../../../components/operation-history/operation-history';
import { HistoryService } from '../../../services/history.service';

@Component({
  selector: 'app-branch-detail',
  standalone: true,
  imports: [CommonModule, OperationHistoryComponent],
  templateUrl: './branch-detail.html',
  styleUrls: ['./branch-detail.css']
})
export class BranchDetailComponent implements OnInit {
  branchId: number = 0;
  products: any[] = [];

  constructor(
    private route: ActivatedRoute,
    private historyService: HistoryService
  ) {}

  ngOnInit(): void {
    this.branchId = +this.route.snapshot.paramMap.get('id')!;
    this.loadProducts();
    this.historyService.fetchAndUpdateHistory(); // Cargar historial inicial
  }

  loadProducts(): void {
    // Tu lógica existente para cargar productos
  }

  onProductAdded(): void {
    this.loadProducts();
    this.historyService.fetchAndUpdateHistory(); // Actualizar historial tras inserción
    this.showToast('Producto agregado correctamente', 'success');
  }

  onProductDeleted(barcode: string): void {
    this.loadProducts();
    this.historyService.fetchAndUpdateHistory(); // Actualizar historial tras eliminación
    this.showToast('Producto eliminado correctamente', 'success');
  }

  onRefreshFromHistory(): void {
    this.loadProducts();
  }

  private showToast(message: string, type: 'success' | 'error'): void {
    const event = new CustomEvent('toast', { detail: { message, type } });
    window.dispatchEvent(event);
  }
}
