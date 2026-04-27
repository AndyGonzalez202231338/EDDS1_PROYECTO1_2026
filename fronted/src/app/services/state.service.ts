import { Injectable, Inject, PLATFORM_ID } from '@angular/core';
import { isPlatformBrowser } from '@angular/common';
import { BehaviorSubject, Observable } from 'rxjs';

@Injectable({ providedIn: 'root' })
export class StateService {
  private readonly KEY = 'selectedBranchId';
  private readonly branchIdSubject: BehaviorSubject<number | null>;
  branchId$: Observable<number | null>;

  constructor(@Inject(PLATFORM_ID) private platformId: object) {
    const initial = isPlatformBrowser(this.platformId) ? this.readFromStorage() : null;
    this.branchIdSubject = new BehaviorSubject<number | null>(initial);
    this.branchId$ = this.branchIdSubject.asObservable();
  }

  setBranchId(id: number | null): void {
    if (isPlatformBrowser(this.platformId)) {
      if (id !== null) {
        localStorage.setItem(this.KEY, String(id));
      } else {
        localStorage.removeItem(this.KEY);
      }
    }
    this.branchIdSubject.next(id);
  }

  getBranchId(): number | null {
    return this.branchIdSubject.value;
  }

  private readFromStorage(): number | null {
    try {
      const v = localStorage.getItem(this.KEY);
      if (v === null) return null;
      const n = parseInt(v, 10);
      return isNaN(n) ? null : n;
    } catch {
      return null;
    }
  }
}
