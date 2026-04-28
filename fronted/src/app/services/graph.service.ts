import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs';
import { Connection } from '../models/connection.model';

export interface PathResult {
  distance: number;
  path: number[];
}

export interface GraphDotResponse {
  dot: string;
  svg: string;
}

@Injectable({ providedIn: 'root' })
export class GraphService {
  constructor(private http: HttpClient) {}

  addEdge(edge: Connection): Observable<{ ok: boolean }> {
    return this.http.post<{ ok: boolean }>('/api/graph/edge', edge);
  }

  getPath(from: number, to: number, criteria: 'time' | 'cost' = 'time'): Observable<PathResult> {
    return this.http.get<PathResult>(`/api/graph/path?from=${from}&to=${to}&criteria=${criteria}`);
  }

  getGraphDot(): Observable<GraphDotResponse> {
    return this.http.get<GraphDotResponse>('/api/graph/dot');
  }
}
