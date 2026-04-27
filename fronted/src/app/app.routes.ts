import { Routes } from '@angular/router';
import { Layout } from './components/layout/layout';
import { BranchListComponent } from './pages/branches/branch-list/branch-list';
import { ProductComponent } from './components/product/product';
import { TransferFormComponent } from './pages/transfer/transfer-form/transfer-form';
import { NetworkGraph } from './pages/graph/network-graph/network-graph';
import { CsvImportComponent } from './pages/csv-import/csv-import';

export const routes: Routes = [
  {
    path: '',
    component: Layout,
    children: [
      { path: '',         redirectTo: 'branches', pathMatch: 'full' },
      { path: 'branches', component: BranchListComponent },
      { path: 'products', component: ProductComponent },
      { path: 'transfer', component: TransferFormComponent },
      { path: 'graph',    component: NetworkGraph },
      { path: 'csv-import', component: CsvImportComponent },
    ],
  },
];
