export interface HistoryEntry {
  type: 'INSERT' | 'REMOVE';
  branchId: number;
  barcode: string;
  description: string;
}

export interface HistoryResponse {
  history: HistoryEntry[];
}

export interface UndoResponse {
  ok: boolean;
  undone: string;
  branchId: number;
  barcode: string;
  error?: string;
}