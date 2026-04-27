export interface Product {
  name: string;
  barcode: string;
  category: string;
  expiry_date: string;
  brand: string;
  price: number;
  stock: number;
  branchId?: number;
}