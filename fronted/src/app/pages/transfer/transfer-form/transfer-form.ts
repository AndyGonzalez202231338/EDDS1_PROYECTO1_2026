import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormBuilder, ReactiveFormsModule, Validators } from '@angular/forms';
import { InventoryService } from '../../../services/inventory.service';

@Component({
  selector: 'app-transfer',
  standalone: true,
  imports: [CommonModule, ReactiveFormsModule],
  templateUrl: './transfer-form.html'
})
export class TransferFormComponent {
  message = '';
  error = '';
  form: any;

  constructor(private fb: FormBuilder, private inventory: InventoryService) {
    this.form = this.fb.group({
      barcode: ['', Validators.required],
      originId: [0, [Validators.required, Validators.min(1)]],
      destId: [0, [Validators.required, Validators.min(1)]]
    });
  }

  submit(): void {
    this.message = '';
    this.error = '';
    if (this.form.invalid) return;

    const v = this.form.value;
    this.inventory.transfer(v.barcode!, Number(v.originId), Number(v.destId)).subscribe({
      next: (res) => {
        if (res.ok) this.message = 'Transferencia realizada';
        else this.error = res.error || 'Transferencia fallida';
      },
      error: (err) => (this.error = err?.error?.error || 'Error en transferencia')
    });
  }
}
