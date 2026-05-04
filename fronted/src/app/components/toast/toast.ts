import { Component, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';

interface Toast {
  id: number;
  message: string;
  type: 'success' | 'error' | 'info';
}

@Component({
  selector: 'app-toast',
  standalone: true,
  imports: [CommonModule],
  template: `
    <div class="toast-container">
      <div *ngFor="let toast of toasts" 
           class="toast" 
           [class]="'toast-' + toast.type">
        <p>{{ toast.message }}</p>
      </div>
    </div>
  `,
  styles: [`
    .toast-container {
      position: fixed;
      bottom: 1rem;
      right: 1rem;
      z-index: 50;
      display: flex;
      flex-direction: column;
      gap: 0.75rem;
    }

    .toast {
      padding: 1rem;
      border-radius: 0.375rem;
      font-weight: 500;
      animation: slideIn 0.3s ease-out;
      min-width: 250px;
    }

    .toast-success {
      background-color: #d1fae5;
      color: #065f46;
      border-left: 4px solid #10b981;
    }

    .toast-error {
      background-color: #fee2e2;
      color: #b91c1c;
      border-left: 4px solid #ef4444;
    }

    .toast-info {
      background-color: #dbeafe;
      color: #1e40af;
      border-left: 4px solid #3b82f6;
    }

    @keyframes slideIn {
      from {
        opacity: 0;
        transform: translateX(100px);
      }
      to {
        opacity: 1;
        transform: translateX(0);
      }
    }
  `]
})
export class ToastComponent implements OnInit {
  toasts: Toast[] = [];
  private toastId = 0;

  ngOnInit(): void {
    window.addEventListener('toast', (event: any) => {
      const { message, type } = event.detail;
      this.addToast(message, type);
    });
  }

  addToast(message: string, type: 'success' | 'error' | 'info'): void {
    const id = this.toastId++;
    this.toasts.push({ id, message, type });
    setTimeout(() => {
      this.toasts = this.toasts.filter(t => t.id !== id);
    }, 4000);
  }
}