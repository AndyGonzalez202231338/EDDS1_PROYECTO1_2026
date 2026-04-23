import { ComponentFixture, TestBed } from '@angular/core/testing';

import { QueueMonitor } from './queue-monitor';

describe('QueueMonitor', () => {
  let component: QueueMonitor;
  let fixture: ComponentFixture<QueueMonitor>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [QueueMonitor]
    })
    .compileComponents();

    fixture = TestBed.createComponent(QueueMonitor);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
