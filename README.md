# STM32 FreeRTOS Data Logger

A high-reliability, multi-task sensor data logger on the STM32 NUCLEO-F446RE (ARM Cortex-M4F) running FreeRTOS. Samples analog input through DMA and digital 6-DOF inertial sensor (MPU6050) over I²C at deterministic rates, writes CRC-32 protected 512-byte blocks to an SD card, features a UART shell, captures HardFault snapshots across reboots, and demonstrates priority inversion diagnosis and mitigation.

---

## 1. Key Highlights & Demonstrated Skills

- **Deterministic Sampling:** Timer-triggered ADC with circular DMA double buffering (1 kHz) and `vTaskDelayUntil()` based I²C sampling (100 Hz).
- **Zero Data Loss Guarantee:** Asynchronous, non-blocking queue between sensor acquisition and SD write task. Saturated queue drops are explicitly counted, never hidden.
- **Power-Loss Durability:** Pre-allocated 16 MB log files (`f_expand`) with 512-byte atomic sector framing and CRC-32 checksums. Survives abrupt power disconnection with automated boot-time recovery.
- **Priority Inversion Benchmark:** Deliberate priority inversion demonstrated between `RecordTask` (High), `LoadTask` (Medium), and `LoggerTask` (Low) under three build modes:
  - **Mode A:** Binary Semaphore (demonstrates severe deadline misses)
  - **Mode B:** Priority-Inheritance Mutex (mitigates medium-task preemption)
  - **Mode C:** Architectural Short Critical Section (eliminates delay completely)
- **HardFault & Watchdog Diagnostics:** Assembly vector stub extracts active stack pointer (MSP vs PSP) during HardFault, saves CPU registers (`R0-R3, R12, LR, PC, xPSR, CFSR, HFSR`) into non-zeroed `.noinit` SRAM, and resolves faulting source lines post-reboot using `arm-none-eabi-addr2line`.
- **Host Unit Testable:** Core modules (`crc32`, `block_builder`, `record_format`, `shell_parser`) run completely on host (x86/ARM) with Unity tests and Python parsers in CI.

---

## 2. Hardware Bill of Materials & Pin Map

| Item | Purpose | Interface |
|---|---|---|
| **NUCLEO-F446RE** | Main MCU (Cortex-M4F @ 180 MHz, 512 KB Flash, 128 KB SRAM) | ST-LINK USB |
| **MPU6050 Module** | 6-DOF IMU (Accelerometer + Gyroscope) | I²C1 (PB8 SCL, PB9 SDA) |
| **MicroSD SPI Module** | 512-byte sector persistent storage | SPI2 (PB12 CS, PB13 SCK, PB14 MISO, PB15 MOSI) |
| **USB Logic Analyzer** | Jitter, latency, and priority inversion measurement | 5x GPIOs (PA8, PA9, PA10, PB0, PB1) |
| **Potentiometer / DAC** | Analog signal input (PA0) / DAC loopback (PA4) | ADC1_IN0 |

Full wiring details are available in [docs/pinmap.md](docs/pinmap.md).

---

## 3. Firmware Architecture

```
Hardware Sources            FreeRTOS Tasks                      Output / Storage
----------------            --------------                      ----------------
TIM2 -> ADC1 (DMA)  ====>   AdcTask (P=5)
                              │  (Latest Slot + Staging Lock)
                              ▼
MPU6050 (I²C)       ====>   RecordTask (P=4)
                              │
                              ▼  (Static Queue, Depth 64)
                            LoggerTask (P=2)  ===============>  SD Card (512B Blocks + CRC-32)

CPU Busy Burst      ====>   LoadTask (P=3)   [Experiment Only]

UART Virtual COM    <===>   ShellTask (P=1)  <===============>  CLI (status, stats, rate, fault)

All Tasks (Bits)    ====>   MonitorTask (P=1) ===============>  IWDG Hardware Refresh (~2s)
```

Detailed design notes and interview rationale are documented in [docs/architecture.md](docs/architecture.md) and [docs/design_notes.md](docs/design_notes.md).

---

## 4. Priority Inversion Benchmark Results

Logic analyzer measurements under `CFG_LOCK_MODE`:

| Mode | Synchronization Primitive | RecordTask Max Wait (ms) | Deadline Misses / 1000 Periods | Description |
|---|---|---|---|---|
| **Mode A** | Binary Semaphore (No Inheritance) | **9.2 ms** | **42** | Severe inversion: LoadTask delays high-priority sampling |
| **Mode B** | FreeRTOS Mutex (Priority Inheritance) | **4.1 ms** | **0** | Inheritance boosts LoggerTask; LoadTask cannot preempt |
| **Mode C** | Mutex + Short Critical Section | **0.05 ms** | **0** | Fast in-memory copy; physical SD write done outside lock |

---

## 5. Host Unit Tests & Python Log Parser

### Running Host C Unit Tests
The core binary framing, CRC calculations, and command parsing logic have zero hardware dependencies and are verified natively on the PC:
```bash
make -C host/tests
```
```text
=== Running Suite: STM32 FreeRTOS Data Logger Host Tests ===
Running test_crc32_known_vector...        [PASS]
Running test_crc32_empty_string...         [PASS]
Running test_crc32_incremental...          [PASS]
Running test_struct_sizes...               [PASS]
Running test_record_packing_roundtrip...   [PASS]
Running test_block_builder_full_block...   [PASS]
Running test_block_builder_flush_partial...[PASS]
Running test_shell_parser_basic...         [PASS]
Running test_shell_parser_empty_and_unknown... [PASS]
=== Results: 9 Passed, 0 Failed ===
```

### Running Log Parser Tests
```bash
python3 host/parser/test_parser.py
```

### Parsing an SD Card Binary Log
```bash
python3 host/parser/log_parser.py log_0001.bin --csv output.csv -v
```

---

## 6. Bare-Metal Learning Ramp Exercises

For developers and interviewers seeking register-level understanding without the HAL:
- [exercises/gpio_bare.c](exercises/gpio_bare.c): Direct register GPIO toggle (`RCC_AHB1ENR`, `GPIOA_MODER`, `GPIOA_BSRR`).
- [exercises/uart_bare.c](exercises/uart_bare.c): Bare-metal USART2 transmission with baud rate calculation (`USART_BRR`).
- [exercises/tim_irq.c](exercises/tim_irq.c): Timer 2 update interrupt and NVIC configuration (`TIM2_PSC`, `TIM2_ARR`, `NVIC_ISER`).
- [exercises/README.md](exercises/README.md): Detailed explanation of Cortex-M reset sequence and NVIC priority inversion rules.

---

## 7. Interactive Shell Commands

Connect to the ST-LINK Virtual COM port at `115200 8N1`:

| Command | Action |
|---|---|
| `help` | Display list of all available commands |
| `status` | Show sample count, dropped samples, blocks written, logging state |
| `stats` | Print per-task CPU load and stack high-water marks |
| `rate <hz>` | Dynamically update sensor rate (10 to 500 Hz) |
| `start` / `stop` | Open new log file / finalize active log file |
| `dump <file>` | Stream binary file as CSV formatted text to terminal |
| `fault <type>` | Provoke intentional fault (`null`, `divzero`, `unaligned`, `invstate`) |
| `wdtest` | Induce task hang to verify independent watchdog reset within 2s |
| `reset` | Trigger clean software reboot via `NVIC_SystemReset()` |
| `version` | Display build timestamp, FreeRTOS version, and active lock mode |
