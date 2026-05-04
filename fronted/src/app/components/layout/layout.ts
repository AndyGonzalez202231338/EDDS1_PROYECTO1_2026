import { Component } from '@angular/core';
import { RouterLink, RouterOutlet } from '@angular/router';
import { CommonModule } from '@angular/common';

@Component({
  selector: 'app-layout',
  standalone: true,
  imports: [CommonModule, RouterLink, RouterOutlet],
  templateUrl: './layout.html',
  styleUrl: './layout.css'
})
export class Layout {
  navItems = [
    { label: 'Sucursales', path: '/branches' },
    { label: 'Productos', path: '/products' },
    { label: 'Transacciones', path: '/transactions' },
    { label: 'Simulación', path: '/simulation' },
    { label: 'Grafo', path: '/graph' },
    { label: 'Importar CSV', path: '/csv-import' },
    { label: 'Benchmark', path: '/benchmark' }
  ];
}
