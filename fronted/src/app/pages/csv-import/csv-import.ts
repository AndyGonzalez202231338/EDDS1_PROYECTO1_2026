import { Component } from '@angular/core';
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

  constructor(private api: ApiService) {}

  clearMessages() {
    this.messages = [];
  }

  addMessage(type: 'success' | 'error' | 'info', text: string) {
    this.messages.push({ type, text });
    if (type === 'error') {
      console.error(text);
    }
  }

  fetchErrorLog() {
    this.api.getCSVErrors().subscribe({
      next: (resp) => {
        console.log('Error log response:', resp);
        if (resp.ok) {
          this.errorLog = resp.errors || 'No hay errores registrados.';
          this.showErrorLog = true;
        } else {
          this.errorLog = 'Error al obtener los logs: ' + (resp.error || 'Desconocido');
          this.showErrorLog = true;
        }
      },
      error: (err) => {
        console.error('Error fetching log:', err);
        this.errorLog = 'Error de red al obtener logs: ' + err.message;
        this.showErrorLog = true;
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
    this.readAndLoadFile(this.branchesFile, 'branches');
  }

  loadConnections() {
    if (!this.connectionsFile) {
      this.addMessage('error', 'Por favor selecciona un archivo de conexiones');
      return;
    }

    this.loading = true;
    this.readAndLoadFile(this.connectionsFile, 'connections');
  }

  loadProducts() {
    if (!this.productsFile) {
      this.addMessage('error', 'Por favor selecciona un archivo de productos');
      return;
    }

    this.loading = true;
    this.readAndLoadFile(this.productsFile, 'products');
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
            this.addMessage('success', `${file.name} cargado exitosamente: ${resp.message}`);
          } else {
            this.addMessage('error', `Error al cargar ${file.name}: ${resp.error || 'Error desconocido'}`);
          }
          // Obtener logs después de cargar
          setTimeout(() => this.fetchErrorLog(), 500);
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
    this.loadBranches();
    setTimeout(() => this.loadConnections(), 1500);
    setTimeout(() => this.loadProducts(), 3000);
    setTimeout(() => this.fetchErrorLog(), 4000);
  }

  toggleErrorLog() {
    this.showErrorLog = !this.showErrorLog;
  }

  clearErrorLog() {
    this.errorLog = '';
    this.showErrorLog = false;
  }
}


