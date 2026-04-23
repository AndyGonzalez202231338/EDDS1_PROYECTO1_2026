import { ComponentFixture, TestBed } from '@angular/core/testing';

import { BranchDetail } from './branch-detail';

describe('BranchDetail', () => {
  let component: BranchDetail;
  let fixture: ComponentFixture<BranchDetail>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [BranchDetail]
    })
    .compileComponents();

    fixture = TestBed.createComponent(BranchDetail);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
