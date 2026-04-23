export interface TransferRequest {
  barcode: string;
  originId: number;
  destId: number;
  criteria: 'time' | 'cost';
}

export interface TransferResult {
  path: number[];
  eta: number;
  totalCost: number;
  status: string;
}
