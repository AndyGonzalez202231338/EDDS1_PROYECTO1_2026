import { Component, OnDestroy, OnInit, ChangeDetectorRef } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RouterLink } from '@angular/router';
import { Subscription } from 'rxjs';
import { skip } from 'rxjs/operators';
import { BranchService } from '../../services/branch.service';
import { StateService } from '../../services/state.service';
import { Branch } from '../../models/branch.model';

@Component({
  selector: 'app-branch-selector',
  standalone: true,
  imports: [CommonModule, RouterLink],
  templateUrl: './branch-selector.html',
})
export class BranchSelectorComponent implements OnInit, OnDestroy {
  branches: Branch[] = [];
  activeBranch: Branch | null = null;
  loading = true;
  private sub?: Subscription;

  constructor(
    private branchService: BranchService,
    private state: StateService,
    private cdr: ChangeDetectorRef
  ) {}

  ngOnInit(): void {
    this.branchService.getBranches().subscribe({
      next: (data) => {
        this.branches = data;
        this.loading = false;
        this.syncActive(this.state.getBranchId());
        this.cdr.markForCheck();
      },
      error: () => {
        this.loading = false;
        this.cdr.markForCheck();
      }
    });

    this.sub = this.state.branchId$.pipe(skip(1)).subscribe((id) => {
      this.syncActive(id);
      this.cdr.markForCheck();
    });
  }

  ngOnDestroy(): void {
    this.sub?.unsubscribe();
  }

  select(id: number): void {
    this.state.setBranchId(id);
  }

  private syncActive(id: number | null): void {
    this.activeBranch = id != null ? (this.branches.find((b) => b.id === id) ?? null) : null;
  }
}
