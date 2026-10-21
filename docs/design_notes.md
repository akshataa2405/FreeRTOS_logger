# Firmware Design Notes & Technical Interview Rationale

This document captures the architectural decisions made in the STM32 FreeRTOS Data Logger to answer senior embedded engineering interview questions.

---

### Q1: Why use Direct Task Notifications instead of a Queue from the DMA ISR to AdcTask?
- **Speed & Memory Overhead:** Direct-to-task notifications (`xTaskNotifyFromISR`) require zero heap and zero dedicated queue memory structure. They write directly into the TCB (Task Control Block) of `AdcTask`.
- **Interrupt Latency:** Sending to a queue copies memory and manipulates queue waiting lists; notifications execute in a fraction of the clock cycles, keeping the ISR execution time under 1 microsecond.

---

### Q2: Why is `MonitorTask` placed at Priority 1 (Lowest Application Priority)?
- If `MonitorTask` ran at high priority, it could continue petting the watchdog even if a lower-priority task (like `LoggerTask` or `ShellTask`) was completely starved of CPU time!
- By running at Priority 1, `MonitorTask` only executes if all higher-priority tasks yield properly. If any task spins in an infinite loop or deadlocks, `MonitorTask` is starved, fails to pet the IWDG, and the hardware resets within 2.0 seconds.

---

### Q3: Why 512-Byte Storage Blocks with CRC-32?
- **Underlying Flash Geometry:** Physical SD cards and SPI flash media organize their internal buffers into 512-byte sectors.
- **Power-Loss Atomicity:** A power cut midway through writing will only ever affect the single in-flight 512-byte sector, never tearing or corrupting across sector boundaries.
- **Data Integrity:** Each block carries its own header and trailing IEEE 802.3 CRC-32. On reboot or during offline python analysis, any corrupted block is instantly caught and isolated without discarding valid preceding data.

---

### Q4: Why Pre-Allocate Log Files (`f_expand`)?
- Standard FAT filesystem writes require periodically updating FAT tables, cluster chains, and directory entries as the file expands.
- If power is pulled while FAT tables are being written, the file allocation table desynchronizes, often corrupting the entire file or leaving it 0 bytes in length.
- Pre-allocating the file to 16 MB marks all clusters in advance. During logging, only data sectors are updated; FAT cluster metadata remains static.

---

### Q5: Why is Mode C (Short Critical Section) better than Mode B (Priority Inheritance)?
- **Priority Inheritance (Mode B)** prevents unbounded priority inversion by boosting the low-priority task (`LoggerTask`) to the priority of the waiting task (`RecordTask`).
- However, `RecordTask` is still forced to wait for `LoggerTask` to finish its slow physical write!
- **Mode C (Architectural Fix)** minimizes the critical section: `LoggerTask` only holds the lock long enough to perform a rapid in-memory structure copy (a few microseconds). It then performs the slow physical write *outside* the lock. This proves that while priority inheritance mitigates damage, clean software architecture eliminates the bottleneck altogether.

---

### Q6: How do you map a HardFault PC to a line of source code?
1. The assembly stub captures the stacked `PC` into `s_fault_storage.pc` in `.noinit` RAM.
2. After reboot, `fault_check_and_report()` prints the hex address, e.g., `PC = 0x08001A42`.
3. Run the toolchain utility with the compiled ELF file:
   ```bash
   arm-none-eabi-addr2line -e logger.elf -f -C 0x08001A42
   ```
   This outputs the exact function name, source filename, and line number that triggered the fault.
