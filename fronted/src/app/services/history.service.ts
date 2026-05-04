import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { BehaviorSubject, Observable } from 'rxjs';
import { HistoryEntry, HistoryResponse, UndoResponse } from '../models/history.model';

@Injectable({
  providedIn: 'root'
})
export class HistoryService {
  private historySubject = new BehaviorSubject<HistoryEntry[]>([]);
  public history$ = this.historySubject.asObservable();

  constructor(private http: HttpClient) {}

  // Obtener historial completo
  getHistory(): Observable<HistoryResponse> {
    return this.http.get<HistoryResponse>('/api/undo/history');
  }

  // Actualizar historial en el BehaviorSubject
  fetchAndUpdateHistory(): void {
    this.getHistory().subscribe({
      next: (response) => {
        this.historySubject.next(response.history || []);
      },
      error: (err) => console.error('Error cargando historial:', err)
    });
  }

  // Ejecutar deshacer
  undo(): Observable<UndoResponse> {
    return this.http.post<UndoResponse>('/api/undo', {});
  }

  // Obtener historial actual
  getCurrentHistory(): HistoryEntry[] {
    return this.historySubject.value;
  }

  // Limpiar historial
  clearHistory(): void {
    this.historySubject.next([]);
  }
}