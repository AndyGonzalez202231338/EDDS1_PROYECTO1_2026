import { Routes } from '@angular/router';
import { Layout } from './components/layout/layout';
import { BranchListComponent } from './pages/branches/branch-list/branch-list';
import { ProductComponent } from './components/product/product';
import { TransferFormComponent } from './pages/transfer/transfer-form/transfer-form';
import { NetworkGraph } from './pages/graph/network-graph/network-graph';
import { CsvImportComponent } from './pages/csv-import/csv-import';
import { SimulationComponent } from './pages/simulation/simulation';
import { Benchmark } from './components/benchmark/benchmark';

export const routes: Routes = [
  {
    path: '',
    component: Layout,
    children: [
      { path: '',              redirectTo: 'branches', pathMatch: 'full' },
      { path: 'branches',      component: BranchListComponent },
      { path: 'products',      component: ProductComponent },
      { path: 'transfer',      component: TransferFormComponent },
      { path: 'simulation',    component: SimulationComponent },
      { path: 'graph',         component: NetworkGraph },
      { path: 'csv-import',    component: CsvImportComponent },
      { path: 'benchmark',     component: Benchmark },
    ],
  },
];
