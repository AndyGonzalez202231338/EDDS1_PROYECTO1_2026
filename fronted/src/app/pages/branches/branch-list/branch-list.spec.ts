import { ComponentFixture, TestBed } from '@angular/core/testing';

import { BranchList } from './branch-list';

describe('BranchList', () => {
  let component: BranchList;
  let fixture: ComponentFixture<BranchList>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [BranchList]
    })
    .compileComponents();

    fixture = TestBed.createComponent(BranchList);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
