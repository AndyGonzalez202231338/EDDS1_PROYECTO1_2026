import { Routes } from '@angular/router';
import { Layout } from './components/layout/layout';
import { Catalog } from './components/catalog/catalog';
import { Benchmark } from './components/benchmark/benchmark';
import { NotFound } from './components/not-found/not-found';

export const routes: Routes = [
  {
    path: '',
    component: Layout,
    children: [
      { path: '', component: Catalog },
      { path: 'benchmark', component: Benchmark },
    ],
  },
  { path: '**', component: NotFound },
];
