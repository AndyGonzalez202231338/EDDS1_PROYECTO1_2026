export interface TransferRequest {
  barcode: string;
  originId: number;
  destId: number;
  type: 'time' | 'cost';
  mode?: 'fast' | 'realtime';
  quantity?: number;
}

export interface TransferResponse {
  ok: boolean;
  error?: string;
  // fast mode
  path?: number[];
  totalTime?: number;
  steps?: string[];
  // realtime mode
  transferId?: string;
}

export type QueueType = 'INGRESO' | 'PREPARACION' | 'SALIDA';

// ── Simulación tick-based ──────────────────────────────────────────────────────

export interface SimQueueEntry {
  transferId: string;
  barcode: string;
  productName: string;
  waitTicks: number;
  etaTicks: number;
}

export interface ProductInQueue {
  barcode: string;
  quantity: number;
  transferId: string;
  productName?: string;
}

export interface BranchSimSnapshot {
  branchId: number;
  colaIngreso: ProductInQueue[];
  colaTraspaso: ProductInQueue[];
  colaSalida: ProductInQueue[];
}

export interface CompletedTransfer {
  transferId: string;
  barcode: string;
  quantity: number;
  completedTick: number;
  originId: number;
  destId: number;
  totalTicks: number;
  ok: boolean;
  error?: string;
}

export interface PendingTransfer {
  id: string;
  barcode: string;
  originId: number;
  destId: number;
  type: 'time' | 'cost';
  quantity: number;
  status: 'pending' | 'executing' | 'completed' | 'failed';
  createdAt: Date;
  error?: string;
}

export interface SimSnapshot {
  currentTick: number;
  branches: BranchSimSnapshot[];
  recentCompleted: CompletedTransfer[];
  activeTransfers?: PendingTransfer[];
}

export interface QueueSnapshot {
  branchId: number;
  queue: QueueType;
  position: number;
  elapsedSeconds: number;
}

// ── Batch simulation ───────────────────────────────────────────────────────────

export interface BatchPendingItem {
  index: number;
  barcode: string;
  originId: number;
  originName: string;
  destId: number;
  destName: string;
  byTime: boolean;
  label: string;
}

export interface BatchPendingResponse {
  pending: BatchPendingItem[];
}

export interface BatchExecuteResultItem {
  barcode: string;
  originId: number;
  destId: number;
  ok: boolean;
  transferId: string;
  error?: string;
}

export interface BatchExecuteResponse {
  executed: number;
  results: BatchExecuteResultItem[];
}

export interface TransferProgress {
  id: string;
  completed: boolean;
  currentBranch: number;
  currentQueue: QueueType;
  totalAccumulated: number;
  steps: string[];
  queues: QueueSnapshot[];
  branches?: BranchSimSnapshot[];
  error?: string;
}
