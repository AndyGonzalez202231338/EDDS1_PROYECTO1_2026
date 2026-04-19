import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs';

export interface Product {
  name: string;
  barcode: string;
  category: string;
  expiry_date: string;
  brand: string;
  price: number;
  stock: number;
}

export interface BenchmarkResult {
  operation: string;
  structure: string;
  caseType: string;
  avgTimeUs: number;
  minTimeUs: number;
  maxTimeUs: number;
}

export interface LoadResult {
  ok: boolean;
  loaded: number;
  errors: number;
}

export interface DotFile {
  name: string;
  url: string;
}

export interface DotResult {
  ok: boolean;
  label: string;
  files: DotFile[];
}

@Injectable({ providedIn: 'root' })
export class ApiService {
  private base = '/api';

  constructor(private http: HttpClient) {}

  ping(): Observable<any> {
    return this.http.get(`${this.base}/ping`);
  }

  // Productos
  getProducts(): Observable<Product[]> {
    return this.http.get<Product[]>(`${this.base}/products`);
  }

  addProduct(p: Product): Observable<any> {
    return this.http.post(`${this.base}/products`, p);
  }

  deleteProduct(barcode: string): Observable<any> {
    return this.http.delete(`${this.base}/products/${barcode}`);
  }

  searchByName(name: string): Observable<Product> {
    return this.http.get<Product>(
      `${this.base}/products/name/${encodeURIComponent(name)}`
    );
  }

  searchByBarcode(barcode: string): Observable<Product> {
    return this.http.get<Product>(
      `${this.base}/products/barcode/${encodeURIComponent(barcode)}`
    );
  }

  searchByCategory(cat: string): Observable<Product[]> {
    return this.http.get<Product[]>(
      `${this.base}/products/category/${encodeURIComponent(cat)}`
    );
  }

  searchByDateRange(d1: string, d2: string): Observable<Product[]> {
    return this.http.get<Product[]>(
      `${this.base}/products/range?d1=${d1}&d2=${d2}`
    );
  }

  // CSV
  loadCSV(path: string): Observable<LoadResult> {
    return this.http.post<LoadResult>(`${this.base}/load`, { path });
  }

  uploadCSV(content: string): Observable<LoadResult> {
    return this.http.post<LoadResult>(`${this.base}/upload`, { content });
  }

  // DOT / árboles
  generateDot(label: string): Observable<DotResult> {
    return this.http.post<DotResult>(`${this.base}/dot`, { label });
  }

  // Errores
  getErrors(): Observable<{ log: string }> {
    return this.http.get<{ log: string }>(`${this.base}/errors`);
  }

  clearErrors(): Observable<any> {
    return this.http.delete(`${this.base}/errors`);
  }

  // Benchmark
  runBenchmark(): Observable<BenchmarkResult[]> {
    return this.http.get<BenchmarkResult[]>(`${this.base}/benchmark`);
  }
}
