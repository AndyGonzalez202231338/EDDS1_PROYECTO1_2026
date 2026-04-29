import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Branch } from '../models/branch.model';
import { Product } from '../models/product.model';
import { Observable } from 'rxjs';

@Injectable({ providedIn: 'root' })
export class BranchService {
  constructor(private http: HttpClient) {}

  getBranches(): Observable<Branch[]> {
    return this.http.get<Branch[]>('/api/branches');
  }

  createBranch(branch: Branch): Observable<{ ok: boolean }> {
    return this.http.post<{ ok: boolean }>('/api/branches', branch);
  }

  deleteBranch(id: number): Observable<{ ok: boolean }> {
    return this.http.delete<{ ok: boolean }>(`/api/branches/${id}`);
  }

  getBranchProducts(branchId: number): Observable<Product[]> {
    return this.http.get<Product[]>(`/api/branch/${branchId}/products`);
  }
}
