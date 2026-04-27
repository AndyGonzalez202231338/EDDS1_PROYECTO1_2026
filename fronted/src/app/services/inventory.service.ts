import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Product } from '../models/product.model';
import { Observable } from 'rxjs';

@Injectable({ providedIn: 'root' })
export class InventoryService {
  constructor(private http: HttpClient) {}

  listProducts(branchId: number): Observable<Product[]> {
    return this.http.get<Product[]>(`/branch/${branchId}/products`);
  }

  insertProduct(branchId: number, product: Product): Observable<{ ok?: boolean; error?: string }> {
    return this.http.post<{ ok?: boolean; error?: string }>(`/branch/${branchId}/product`, product);
  }

  searchByBarcode(branchId: number, barcode: string): Observable<Product> {
    return this.http.get<Product>(`/branch/${branchId}/product/${encodeURIComponent(barcode)}`);
  }

  removeProduct(branchId: number, barcode: string): Observable<{ ok?: boolean; error?: string }> {
    return this.http.delete<{ ok?: boolean; error?: string }>(`/branch/${branchId}/product/${encodeURIComponent(barcode)}`);
  }

  transfer(barcode: string, originId: number, destId: number) {
    return this.http.post<{ ok: boolean; error?: string }>('/api/transfer', {
      barcode,
      originId,
      destId
    });
  }
}