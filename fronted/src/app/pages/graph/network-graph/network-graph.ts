import { Component, OnInit, ViewChild, ElementRef, ChangeDetectorRef } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ReactiveFormsModule, FormBuilder, FormGroup, Validators } from '@angular/forms';
import { GraphService, PathResult } from '../../../services/graph.service';
import { BranchService } from '../../../services/branch.service';
import { Branch } from '../../../models/branch.model';
import { DomSanitizer, SafeHtml } from '@angular/platform-browser';

@Component({
  selector: 'app-network-graph',
  standalone: true,
  imports: [CommonModule, ReactiveFormsModule],
  templateUrl: './network-graph.html',
  styleUrl: './network-graph.css',
})
export class NetworkGraph implements OnInit {
  @ViewChild('graphContainer', { static: false }) graphContainer?: ElementRef;

  branches: Branch[] = [];
  pathResult: PathResult | null = null;
  graphSvg: SafeHtml | null = null;

  message = '';
  error = '';
  connecting = false;
  calculating = false;
  loadingGraph = false;

  edgeForm: FormGroup;
  pathForm: FormGroup;

  constructor(
    private fb: FormBuilder,
    private graphService: GraphService,
    private branchService: BranchService,
    private sanitizer: DomSanitizer,
    private cdr: ChangeDetectorRef
  ) {
    this.edgeForm = this.fb.group({
      originId:      [null, [Validators.required, Validators.min(1)]],
      destId:        [null, [Validators.required, Validators.min(1)]],
      tiempo:        [1,    [Validators.required, Validators.min(0.1)]],
      costo:         [1,    [Validators.required, Validators.min(0)]],
      bidirectional: [true],
    });

    this.pathForm = this.fb.group({
      from:     [null, [Validators.required, Validators.min(1)]],
      to:       [null, [Validators.required, Validators.min(1)]],
      criteria: ['time'],
    });
  }

  ngOnInit(): void {
    this.branchService.getBranches().subscribe({
      next: (data) => {
        this.branches = data;
        this.cdr.markForCheck();
      },
    });
    this.loadGraph();
  }

  loadGraph(): void {
    this.loadingGraph = true;
    this.graphSvg = null;
    this.cdr.markForCheck();
    this.graphService.getGraphDot().subscribe({
      next: (res) => {
        if (res.svg) {
          const processed = res.svg
            .replace(/(<svg[^>]*)\swidth="[^"]*pt"/, '$1 width="100%"')
            .replace(/(<svg[^>]*)\sheight="[^"]*pt"/, '$1 height="auto"');
          this.graphSvg = this.sanitizer.bypassSecurityTrustHtml(processed);
        }
        this.loadingGraph = false;
        this.cdr.markForCheck();
      },
      error: (err) => {
        console.error('Error loading graph:', err);
        this.error = 'Error al cargar el grafo';
        this.loadingGraph = false;
        this.cdr.markForCheck();
      },
    });
  }

  branchName(id: number): string {
    const b = this.branches.find((b) => b.id === id);
    return b ? b.nombre : `#${id}`;
  }

  connectBranches(): void {
    if (this.edgeForm.invalid) { this.edgeForm.markAllAsTouched(); return; }
    this.connecting = true;
    this.clearMessages();

    this.graphService.addEdge(this.edgeForm.value).subscribe({
      next: () => {
        this.message = `Conexión ${this.edgeForm.value.originId} ↔ ${this.edgeForm.value.destId} creada`;
        this.connecting = false;
        this.edgeForm.reset({ bidirectional: true });
        this.cdr.markForCheck();
        this.loadGraph();
      },
      error: (err) => {
        this.error = err?.error?.error || 'Error al crear la conexión';
        this.connecting = false;
        this.cdr.markForCheck();
      },
    });
  }

  calcPath(): void {
    if (this.pathForm.invalid) { this.pathForm.markAllAsTouched(); return; }
    this.calculating = true;
    this.pathResult = null;
    this.clearMessages();

    const { from, to, criteria } = this.pathForm.value;
    this.graphService.getPath(from, to, criteria).subscribe({
      next: (res) => {
        this.pathResult = res;
        this.calculating = false;
        this.cdr.markForCheck();
      },
      error: (err) => {
        this.error = err?.error?.error || 'No hay ruta entre esas sucursales';
        this.calculating = false;
        this.cdr.markForCheck();
      },
    });
  }

  private clearMessages(): void {
    this.message = '';
    this.error = '';
    this.pathResult = null;
  }
}
