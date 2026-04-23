export interface Connection {
  originId: number;
  destId: number;
  tiempo: number;
  costo: number;
  bidirectional: boolean;
}