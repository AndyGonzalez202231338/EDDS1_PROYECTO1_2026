import { Component, Input, Output, EventEmitter, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';
import { HistoryService } from '../../services/history.service';
import { HistoryEntry } from '../../models/history.model';

@Component({
  selector: 'app-operation-history',
  standalone: true,
  imports: [CommonModule],
  templateUrl: './operation-history.html',
  styleUrls: ['./operation-history.css']
})
export class OperationHistoryComponent implements OnInit {
  @Input() branchId: number = 0;
  @Output() refreshProducts = new EventEmitter<void>();

  history: HistoryEntry[] = [];
  isExpanded = false;
  isUndoLoading = false;
  errorMessage: string | null = null;

  constructor(private historyService: HistoryService) {}

  ngOnInit(): void {
    this.loadHistory();
    // Suscribirse a cambios de historial
    this.historyService.history$.subscribe((h) => {
      this.history = h;
    });
  }

  loadHistory(): void {
    this.historyService.fetchAndUpdateHistory();
  }

  handleUndo(): void {
    this.isUndoLoading = true;
    this.errorMessage = null;

    this.historyService.undo().subscribe({
      next: (response) => {
        if (response.ok) {
          this.showToast(`Operación deshecha: ${response.undone}`, 'success');
          this.loadHistory();
          this.refreshProducts.emit();
        } else {
          this.errorMessage = response.error || 'Error al deshacer';
          this.showToast(this.errorMessage, 'error');
        }
        this.isUndoLoading = false;
      },
      error: (err) => {
        this.errorMessage = 'Error al deshacer la operación';
        this.showToast(this.errorMessage, 'error');
        this.isUndoLoading = false;
      }
    });
  }

  getIcon(type: string): string {
    return type === 'INSERT' ? 'agregar' : 'eliminar';
  }

  getIconClass(type: string): string {
    return type === 'INSERT' ? 'icon-insert' : 'icon-remove';
  }

  toggleExpanded(): void {
    this.isExpanded = !this.isExpanded;
  }

  private showToast(message: string, type: 'success' | 'error' | 'info'): void {
    const event = new CustomEvent('toast', { detail: { message, type } });
    window.dispatchEvent(event);
  }
}