import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs';
import { 
  SimSnapshot, 
  TransferResponse, 
  TransferRequest,
  BatchPendingResponse,
  BatchExecuteResponse,
  TransferProgress
} from '../models/transfer.model';

@Injectable({
  providedIn: 'root'
})
export class TransferService {
  private apiUrl = '/api';

  constructor(private http: HttpClient) {}

  transfer(req: TransferRequest): Observable<TransferResponse> {
    return this.http.post<TransferResponse>('/api/transfer', req);
  }

  getStatus(transferId: string): Observable<TransferProgress> {
    return this.http.get<TransferProgress>(`/api/transfer/status/${transferId}`);
  }

  startSimTransfer(req: TransferRequest): Observable<{ ok: boolean; transferId?: string; error?: string }> {
    return this.http.post<{ ok: boolean; transferId?: string; error?: string }>('/api/sim/transfer', req);
  }

  getSimState(): Observable<SimSnapshot> {
    return this.http.get<SimSnapshot>(`${this.apiUrl}/transfers/sim-state`);
  }

  addToBatch(req: TransferRequest): Observable<{ ok: boolean; pendingCount?: number; error?: string }> {
    return this.http.post<{ ok: boolean; pendingCount?: number; error?: string }>('/api/sim/batch/add', req);
  }

  getBatch(): Observable<BatchPendingResponse> {
    return this.http.get<BatchPendingResponse>('/api/sim/batch');
  }

  clearBatch(): Observable<{ ok: boolean }> {
    return this.http.delete<{ ok: boolean }>('/api/sim/batch');
  }

  executeBatch(): Observable<BatchExecuteResponse> {
    return this.http.post<BatchExecuteResponse>('/api/sim/batch/execute', {});
  }

  startTransfer(request: TransferRequest): Observable<TransferResponse> {
    return this.http.post<TransferResponse>(`${this.apiUrl}/transfers/start`, request);
  }

  startMultiTransfers(transfers: any[]): Observable<{ ok: boolean; successCount?: number; error?: string }> {
    return this.http.post<{ ok: boolean; successCount?: number; error?: string }>(
      `${this.apiUrl}/transfers/multi-start`,
      { transfers }
    );
  }
}
