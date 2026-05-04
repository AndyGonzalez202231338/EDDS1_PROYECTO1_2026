export interface Product {
  name: string;
  barcode: string;
  category: string;
  expiry_date: string;
  brand: string;
  price: number;
  stock: number;
  status?: 'AVAILABLE' | 'IN_TRANSIT' | 'DEPLETED';
  branchId?: number;
  timeUs?: number;
}