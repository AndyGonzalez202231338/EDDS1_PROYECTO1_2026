import { ComponentFixture, TestBed } from '@angular/core/testing';

import { Benchmark } from './benchmark';

describe('Benchmark', () => {
  let component: Benchmark;
  let fixture: ComponentFixture<Benchmark>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [Benchmark]
    })
    .compileComponents();

    fixture = TestBed.createComponent(Benchmark);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
