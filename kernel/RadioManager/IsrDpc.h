#ifndef _PCIE_INTERRUPT_H_
#define _PCIE_INTERRUPT_H_

NTSTATUS PCIEInterruptCreate(IN PDEVICE_EXTENSION pDevExt);

BOOLEAN PCIEEvtInterruptIsr(IN WDFINTERRUPT Interrupt, IN ULONG MessageID);

VOID PCIEEvtInterruptDpc(IN WDFINTERRUPT Interrupt,IN WDFDEVICE Device);

NTSTATUS PCIEEvtInterruptEnable(
            IN WDFINTERRUPT Interrupt,
            IN WDFDEVICE    Device
            );

NTSTATUS PCIEEvtInterruptDisable(
            IN WDFINTERRUPT Interrupt,
            IN WDFDEVICE    Device
            );

#endif