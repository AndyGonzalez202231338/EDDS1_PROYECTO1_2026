import { Component, ChangeDetectorRef } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { ApiService } from '../../services/api.service';

@Component({
  selector: 'app-csv-import',
  standalone: true,
  imports: [CommonModule, FormsModule],
  templateUrl: './csv-import.html',
  styleUrls: ['./csv-import.css']
})
export class CsvImportComponent {
  branchesFile: File | null = null;
  connectionsFile: File | null = null;
  productsFile: File | null = null;

  loading = false;
  messages: { type: string; text: string }[] = [];
  errorLog = '';
  showErrorLog = false;

  constructor(private api: ApiService, private cdr: ChangeDetectorRef) {}

  clearMessages() {
    this.messages = [];
  }

  addMessage(type: 'success' | 'error' | 'info', text: string) {
    this.messages.push({ type, text });
    if (type === 'error') {
      console.error(text);
    }
    this.cdr.markForCheck();
  }

  fetchErrorLog() {
    this.api.getErrors().subscribe({
      next: (resp) => {
        this.errorLog = resp.log || '';
        this.showErrorLog = true;
        this.cdr.markForCheck();
      },
      error: (err) => {
        console.error('Error fetching log:', err);
        this.errorLog = 'Error de red al obtener logs: ' + err.message;
        this.showErrorLog = true;
        this.cdr.markForCheck();
      }
    });
  }

  onBranchesFileSelected(event: any) {
    const file = event.target.files?.[0];
    if (file) {
      if (file.name.endsWith('.csv')) {
        this.branchesFile = file;
        this.addMessage('info', `Archivo seleccionado: ${file.name}`);
      } else {
        this.addMessage('error', 'Por favor selecciona un archivo CSV');
        event.target.value = '';
      }
    }
  }

  onConnectionsFileSelected(event: any) {
    const file = event.target.files?.[0];
    if (file) {
      if (file.name.endsWith('.csv')) {
        this.connectionsFile = file;
        this.addMessage('info', `Archivo seleccionado: ${file.name}`);
      } else {
        this.addMessage('error', 'Por favor selecciona un archivo CSV');
        event.target.value = '';
      }
    }
  }

  onProductsFileSelected(event: any) {
    const file = event.target.files?.[0];
    if (file) {
      if (file.name.endsWith('.csv')) {
        this.productsFile = file;
        this.addMessage('info', `Archivo seleccionado: ${file.name}`);
      } else {
        this.addMessage('error', 'Por favor selecciona un archivo CSV');
        event.target.value = '';
      }
    }
  }

  uploadBranchesFile(inputElement: any) {
    inputElement.click();
  }

  uploadConnectionsFile(inputElement: any) {
    inputElement.click();
  }

  uploadProductsFile(inputElement: any) {
    inputElement.click();
  }

  loadBranches() {
    if (!this.branchesFile) {
      this.addMessage('error', 'Por favor selecciona un archivo de sucursales');
      return;
    }
    this.loading = true;
    this.clearLogAndLoad(this.branchesFile, 'branches');
  }

  loadConnections() {
    if (!this.connectionsFile) {
      this.addMessage('error', 'Por favor selecciona un archivo de conexiones');
      return;
    }
    this.loading = true;
    this.clearLogAndLoad(this.connectionsFile, 'connections');
  }

  loadProducts() {
    if (!this.productsFile) {
      this.addMessage('error', 'Por favor selecciona un archivo de productos');
      return;
    }
    this.loading = true;
    this.clearLogAndLoad(this.productsFile, 'products');
  }

  private clearLogAndLoad(file: File, type: 'branches' | 'connections' | 'products') {
    this.api.clearErrors().subscribe({
      next: () => this.readAndLoadFile(file, type),
      error: () => this.readAndLoadFile(file, type),
    });
  }

  private readAndLoadFile(file: File, type: 'branches' | 'connections' | 'products') {
    const reader = new FileReader();

    reader.onload = (e: any) => {
      const content = e.target.result;

      let apiCall;
      if (type === 'branches') {
        apiCall = this.api.loadBranchesCSVContent(content);
      } else if (type === 'connections') {
        apiCall = this.api.loadConnectionsCSVContent(content);
      } else {
        apiCall = this.api.loadProductsCSVContent(content);
      }

      apiCall.subscribe({
        next: (resp) => {
          this.loading = false;
          if (resp.ok) {
            const errorCount = resp.errors ? ` (${resp.errors} errores)` : '';
            this.addMessage('success', `${file.name} cargado exitosamente${errorCount}`);
          } else {
            this.addMessage('error', `Error al cargar ${file.name}: ${resp.error || 'Error desconocido'}`);
          }
          setTimeout(() => this.fetchErrorLog(), 300);
        },
        error: (err) => {
          this.loading = false;
          this.addMessage('error', `Error de red: ${err.message}`);
          this.fetchErrorLog();
        }
      });
    };

    reader.onerror = () => {
      this.loading = false;
      this.addMessage('error', 'Error al leer el archivo');
    };

    reader.readAsText(file);
  }

  loadAll() {
    this.clearMessages();
    if (!this.branchesFile || !this.connectionsFile || !this.productsFile) {
      this.addMessage('error', 'Por favor selecciona todos los archivos antes de cargar');
      return;
    }
    this.addMessage('info', 'Iniciando carga de datos...');
    // Limpiar log una sola vez al inicio y luego cargar secuencialmente
    this.api.clearErrors().subscribe({
      next: () => {
        this.errorLog = '';
        this.showErrorLog = false;
        this.readAndLoadFile(this.branchesFile!, 'branches');
        setTimeout(() => this.readAndLoadFile(this.connectionsFile!, 'connections'), 1500);
        setTimeout(() => this.readAndLoadFile(this.productsFile!, 'products'), 3000);
        setTimeout(() => this.fetchErrorLog(), 4000);
      },
      error: () => {
        this.readAndLoadFile(this.branchesFile!, 'branches');
        setTimeout(() => this.readAndLoadFile(this.connectionsFile!, 'connections'), 1500);
        setTimeout(() => this.readAndLoadFile(this.productsFile!, 'products'), 3000);
        setTimeout(() => this.fetchErrorLog(), 4000);
      }
    });
  }

  toggleErrorLog() {
    this.showErrorLog = !this.showErrorLog;
  }

  clearErrorLog() {
    this.api.clearErrors().subscribe();
    this.errorLog = '';
    this.showErrorLog = false;
    this.cdr.markForCheck();
  }
}
