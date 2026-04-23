import { ComponentFixture, TestBed } from '@angular/core/testing';

import { BranchForm } from './branch-form';

describe('BranchForm', () => {
  let component: BranchForm;
  let fixture: ComponentFixture<BranchForm>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [BranchForm]
    })
    .compileComponents();

    fixture = TestBed.createComponent(BranchForm);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
