# PRD: STM32 FreeRTOS Data Logger

Oct 21, 2026 · @Bramha

## 1. Overview

This project is a small multi-task data logger on an STM32 (ARM Cortex-M) board running FreeRTOS. It samples an analog input through DMA and a digital sensor over I²C at fixed rates, stores timestamped records on an SD card, offers a UART command shell, and survives faults and power loss. One student builds it alone in four weeks.

**Problem.** The builder's other two projects (an ESP32 monitor and a C++ server) do not show the skills most entry-level embedded interviews test: ARM Cortex-M peripherals, interrupts and DMA, an RTOS, timing analysis and fault handling. Typical student RTOS projects blink LEDs from two tasks and stop there.

**Vision.** A logger whose behaviour is measured, not assumed: task run times, CPU load, stack use, sampling jitter, a deliberately created and fixed priority inversion, and a proven recovery after power loss.

**Why this project**

- It covers the standard embedded interview topics in one place: timers, ADC with DMA, I²C, SPI, UART, interrupts, queues, mutexes, watchdogs and hard faults.
- It is small: one board, a few cheap parts, no custom PCB.
- Every claim on the resume comes with a measurement the builder can show.
- It is explainable. Each part (timer to DMA to task to queue to storage) has a clear job.

## 2. Goals, non-goals and success criteria

The project succeeds if the logger runs for hours without losing data, and the builder can show measurements for every claim.

**Goals**

- G1: Sample one analog channel (timer-triggered ADC with DMA) and one I²C sensor at fixed, configurable rates.
- G2: Log timestamped records to an SD card (or SPI flash) through a dedicated low-priority task, without ever blocking sampling.
- G3: Provide a UART command shell for status, configuration and data download.
- G4: Detect task hangs with a watchdog fed only when every task has checked in.
- G5: Capture hard-fault information and report it after the reboot.
- G6: Demonstrate a real priority inversion on purpose, measure it, then fix it with a priority-inheritance mutex.
- G7: Prove recovery after power loss: the log stays readable and loses at most the last unsaved block.
- G8: Measure and report timing (jitter), CPU load, stack use and heap use.

**Non-goals**

- A custom PCB, wireless links, a GUI, or a cloud backend.
- Sample rates above the limits of the chosen sensor and SD card.
- Safety certification or production use.
- Porting to multiple boards. One board, documented well.

**Success criteria**

| ID | Criterion | Target |
| --- | --- | --- |
| S1 | Sampling period jitter at 100 Hz, measured on a logic analyzer | Under 1% peak-to-peak of the period (hypothesis; report the measured value) |
| S2 | Samples lost during a 1-hour run | 0, verified by sequence numbers |
| S3 | Power-loss recovery over repeated tests | At least 50 pulls with no unreadable log and no gap before the last block |
| S4 | Priority inversion demo | Worst-case delay of the high-priority task shown before and after the fix, with the fix removing the extra delay |
| S5 | Fault handler | A deliberate null-pointer access reboots the board and the next boot prints the saved fault registers |
| S6 | Watchdog | A deliberately hung task causes a reset within the watchdog timeout |
| S7 | Builder can explain every task, priority and queue without notes | Mock interview passes |

## 3. Users and use cases

| Persona | Who | Needs | How the logger serves them |
| --- | --- | --- | --- |
| Builder | Third-year electronics student with ESP32 and Verilog experience, new to STM32 and FreeRTOS | An embedded project with RTOS, DMA and fault handling that survives deep interview questions | Each feature maps to a standard interview topic and ships with a measurement |
| Interviewer | Embedded engineer reviewing a repo or a demo in five minutes | Quick evidence of real firmware skill | Short demo, logic-analyzer screenshots, a results table, clean modular code |
| Lab user (hypothetical) | Someone logging a sensor in a lab or field | Reliable, timestamped data that survives power loss | Plug in, press start, pull the card, read a CSV |

**Primary use cases**

1. The user powers the board, inserts an SD card, and logging starts automatically with a status LED pattern.
2. The user opens a serial terminal, runs `status` and sees uptime, sample count, dropped samples, CPU load and free stack per task.
3. The user changes the sampling rate with `rate 200` without rebooting; the logger applies it at the next block boundary.
4. The user pulls power during logging, powers back on, and the logger reports how many records it recovered.
5. A task hangs because of a bug; the watchdog resets the board and the next boot reports which task failed to check in.
6. A fault occurs (for example an invalid memory access); the next boot prints the saved registers so the cause can be traced to a line of code.

## 4. Hardware and bill of materials

Prices are rough Indian-market estimates and must be verified before buying. Total is expected to be roughly 3,000 to 5,000 INR if the college lends no equipment.

| Item | Purpose | Key specs and notes | Qty | Approx. cost (INR) | Alternatives |
| --- | --- | --- | --- | --- | --- |
| NUCLEO-F446RE board | Main MCU board | Cortex-M4F; on-board ST-LINK debugger and USB virtual COM port; 12-bit ADC, DMA, I²C, SPI, UART, independent watchdog; has a DAC, which allows a self-generated test signal (verify on the datasheet) | 1 | 1,800 to 3,000 | NUCLEO-F411RE (no DAC; use a function generator), NUCLEO-F103RB (older core, cheaper) |
| MPU6050 module | I²C sensor sampled at a fixed rate | 3.3 V logic, accel and gyro, I²C address 0x68 | 1 | 150 to 300 | BMP280 or any I²C sensor with a documented register map |
| microSD SPI module and card | Log storage | Use a 3.3 V compatible module; card up to 32 GB formatted FAT32 | 1 | 150 to 350 | SPI flash chip (W25Q series) with a custom record format |
| 8-channel USB logic analyzer | Measure jitter, task timing and the priority inversion | Saleae-compatible clone with PulseView (sigrok) works | 1 | 400 to 800 | College oscilloscope with a GPIO toggle |
| 10 kΩ potentiometer | Slow analog input for first tests | Wiper to an ADC pin | 1 | 20 to 50 | Onboard DAC loopback |
| Breadboard, jumper wires, LEDs, resistors, push button | Wiring, status LEDs, manual input | Standard | 1 set | 200 to 400 | None |
| USB cable with an inline switch, or a battery pack with a switch | Power-loss test rig | Lets power be cut abruptly and repeatably | 1 | 100 to 300 | A GPIO-driven relay for automated pulls |

**Software and tools**

- STM32CubeIDE (compiler, debugger, and CubeMX configuration) or CubeMX with VS Code and CMake.
- FreeRTOS, using its native API for clarity (the CMSIS wrapper hides the names interviewers ask about).
- STM32 HAL for fast setup, plus one peripheral driver written at register level to learn how the hardware works (see section 5).
- FatFs (ChaN) for the SD card.
- A serial terminal (PuTTY, minicom or pyserial), PulseView for the logic analyzer, Python 3 for the log parser and CSV converter.
- Git and a GitHub Actions workflow that builds the firmware on every push.
- Optional trace tool such as SEGGER SystemView or Percepio Tracealyzer (check the license terms before use).

**Pin map.** The builder documents every pin used (ADC input, I²C, SPI chip select, UART, status LED, and three debug GPIOs for the logic analyzer) in the README before wiring.

## 5. Scope and phases

The builder is new to ARM Cortex-M, so the plan opens with a short learning ramp and uses the vendor HAL (hardware abstraction layer) to move quickly. Every phase ends with something that runs on the board.

| Tier | Contents | Done when |
| --- | --- | --- |
| Learning ramp (first 3 days of week 1) | Read the Cortex-M4 reset and boot sequence, vector table, NVIC priorities and SysTick. Write three small register-level exercises with no HAL: toggle a GPIO, send characters over UART, and run a timer interrupt. Step through each in the debugger. | The builder can explain, without notes, what happens from reset to `main()` and how an interrupt reaches its handler |
| MVP (rest of week 1) | CubeMX project with FreeRTOS, two blinking tasks, `printf` over UART, timer-triggered ADC with DMA into a double buffer, sequence-numbered samples counted in a task | Sample counter on the terminal matches the expected rate; no samples lost over 10 minutes |
| v1 (weeks 2 and 3) | I²C sensor task, queue to a logger task, SD logging with FatFs and CRC-protected records, UART command shell, hardware watchdog with a task check-in scheme, HardFault capture and reset-cause reporting, runtime statistics (CPU load and stack high-water marks) | An hour-long run logs every sample; shell commands work; watchdog catches an induced hang |
| v2 (week 4) | Priority inversion experiment and fix, power-loss test campaign, jitter measurement with the logic analyzer, benchmark and results write-up, README and demo video | All success criteria in section 2 met |
| Stretch | DMA-based UART receive with idle-line detection, sleep-mode power measurement, a Python plotting tool for the logs, a second sensor | Only if v2 finishes early |

**Cut order if time runs short:** drop stretch first, then shell extras, then the second sensor. If FatFs and the SD card cost more than two days, switch to a raw SPI flash chip with the record format in section 9. Never cut the measurements, the priority inversion experiment, the fault handler or the power-loss test; those are what make the project stand out.

## 6. Functional requirements

Priority: M = must, S = should, C = could.

**Sampling**

| ID | Requirement | Pri | Acceptance criteria |
| --- | --- | --- | --- |
| FR-001 | Sample one ADC channel at a configurable rate (default 1 kHz) using a timer trigger and DMA into a double buffer, with no per-sample CPU work | M | Logic analyzer shows the DMA half-transfer interrupt at the expected period; CPU load of the sampling path is reported |
| FR-002 | Read the I²C sensor at a configurable rate (default 100 Hz, range 10 to 200 Hz) from a dedicated task using absolute-time delays | M | Measured period jitter meets success criterion S1 |
| FR-003 | Stamp every record with a 32-bit sequence number and a microsecond timestamp from a free-running hardware timer, with wrap-around handled | M | Over a 1-hour run, sequence numbers are contiguous and timestamps are monotonic |
| FR-004 | Detect and count overruns (DMA overrun, full queue); never drop data silently | M | Induced overrun increments the counter and appears in `status` |
| FR-005 | Combine the ADC block and the sensor reading into one record per sensor tick (ADC mean, minimum and maximum over the interval, plus sensor axes) | M | Record contents match a known test signal from the DAC loopback |

**Logging**

| ID | Requirement | Pri | Acceptance criteria |
| --- | --- | --- | --- |
| FR-010 | A logger task receives records through a queue (default depth 64) and writes 512-byte blocks to the SD card | M | Hour-long run produces a file whose record count equals the sequence-number span |
| FR-011 | Each block carries a header (magic, version, first sequence number, record count) and a CRC-32 over the block | M | A corrupted block is detected by the Python parser |
| FR-012 | The logger must never block sampling. If the queue is full, the newest record is dropped and counted | M | Test with the SD card slowed down: sampling period unchanged, drops counted |
| FR-013 | Start a new numbered log file on `start` and after a configurable size (default 16 MB) | S | Files `log_0001.bin`, `log_0002.bin` appear as expected |
| FR-014 | At boot, scan the newest file, find the last valid block, report the number of recovered records, and continue in a new file | M | After a power pull, the boot message states the recovered count and the parser reads every earlier block without error |
| FR-015 | If the SD card is missing or fails, keep sampling, show a distinct LED pattern, and retry mounting every 2 seconds | S | Removing and reinserting the card during a run resumes logging in a new file |

**UART shell**

| ID | Requirement | Pri | Acceptance criteria |
| --- | --- | --- | --- |
| FR-020 | Interactive shell on the USB virtual COM port, 115200 baud 8N1, with `help` | M | Commands respond within 100 ms on an idle system |
| FR-021 | Commands: `status`, `stats`, `rate <hz>`, `start`, `stop`, `ls`, `dump <file>` (CSV output), `fault`, `wdtest`, `reset`, `version` | M for the first five, S for the rest | Each command is covered by a test script using pyserial |
| FR-022 | The shell runs at a lower priority than sampling and logging and never delays them | M | Flooding the shell with commands does not change sampling jitter beyond the S1 limit |

**Reliability and fault handling**

| ID | Requirement | Pri | Acceptance criteria |
| --- | --- | --- | --- |
| FR-030 | Hardware independent watchdog (about 2 s timeout). A monitor task feeds it only when every task has checked in during the last period (event-group bits) | M | A task made to hang on purpose (`wdtest`) causes a reset within the timeout; the next boot names the missing task |
| FR-031 | Read and report the reset cause (power-on, pin, software, watchdog, brown-out) at boot | M | Each cause is triggered once and reported correctly |
| FR-032 | HardFault handler saves stacked registers (R0 to R3, R12, LR, PC, xPSR) and fault status registers (CFSR, HFSR, MMFAR, BFAR) to a no-init RAM region, then resets; the next boot prints and logs them and clears the region | M | A deliberate invalid memory access is traced to the correct source line using the saved PC and the map file |
| FR-033 | Enable FreeRTOS stack-overflow checking and the allocation-failure hook; record the offending task | S | An induced overflow is caught and reported by name |

**Instrumentation**

| ID | Requirement | Pri | Acceptance criteria |
| --- | --- | --- | --- |
| FR-040 | Per-task CPU load using a dedicated hardware timer as the runtime-statistics clock | M | `stats` prints a table that sums to about 100% |
| FR-041 | Report each task's minimum free stack (high-water mark) and the free heap | M | Values appear in `stats` and in the README table |
| FR-042 | Toggle debug GPIO pins at key points (sample start and end, sensor read start and end, logger write start and end, shared-resource lock and unlock) | M | All events visible on the logic analyzer with labels in the PulseView session file |
| FR-043 | A build option selects the shared-resource primitive for the priority inversion experiment: mode A uses a binary semaphore without priority inheritance, mode B uses a mutex with priority inheritance | M | Both modes build and run; results recorded in section 10's table |

## 7. Non-functional requirements

Targets are starting hypotheses. The builder measures first, then reports the real values with the board, clock speed, compiler flags and card used. A missed target is explained, not hidden.

| ID | Area | Requirement | How measured |
| --- | --- | --- | --- |
| NFR-01 | Timing | Sensor-task period jitter under 1% of the period at 100 Hz | Logic analyzer on the debug GPIO, 10,000 periods, report mean, standard deviation and maximum |
| NFR-02 | Latency | From DMA half-transfer interrupt to the record entering the queue under 1 ms | Debug GPIO pair |
| NFR-03 | Reliability | Zero lost samples in a 1-hour run at default rates | Sequence-number check in the Python parser |
| NFR-04 | Durability | After 50 abrupt power pulls, every log is parseable and the loss is at most the last partial block | Automated parser report per pull |
| NFR-05 | CPU load | Total CPU load below 40% at default rates, with a per-task table | Runtime statistics |
| NFR-06 | Memory | Every task has at least 25% of its stack free at the worst case; heap use reported | High-water marks after a stress run |
| NFR-07 | Watchdog | Reset within 2.5 s of a task hang | Logic analyzer or timestamps on the UART |
| NFR-08 | Boot time | Time from reset to the first sample logged under 3 s with a card present | Debug GPIO and terminal timestamps |
| NFR-09 | Code quality | No compiler warnings with `-Wall -Wextra`; static analysis (cppcheck) clean or justified | CI job |
| NFR-10 | Maintainability | One module per concern (sampling, logger, shell, fault, config); each header states its threading rules (which task owns what) | Code review checklist |
| NFR-11 | Reproducibility | A new reader can build, flash and run the demo from the README in under 30 minutes | Dry run by another person |
| NFR-12 | Safety margins | No dynamic allocation after startup; all queue, task and buffer sizes fixed at build time | Code search and heap statistics |

**Note.** The jitter and CPU targets assume the default configuration. If the builder raises the sampling rate, the targets are recalculated and published with the new configuration.

## 8. Firmware architecture

&#91;embedded content: firmware architecture · 6 tasks, 3 hardware sources\]

The DMA interrupt wakes AdcTask, which hands its latest block to RecordTask; RecordTask adds the sensor reading and sends one record to LoggerTask through a queue, and LoggerTask writes CRC-protected blocks to the SD card. The dashed LoadTask exists only in the priority-inversion builds. Fault capture and reset-cause reporting happen at boot and are covered in section 10.

## 9. Firmware design

In FreeRTOS a larger priority number means a more urgent task, and the idle task is priority 0. On Cortex-M the opposite holds for interrupts: a smaller number means more urgent. Mixing these up is a classic bug the builder must be able to explain.

**Tasks** (all created with static allocation)

| Task | Priority | Trigger | Stack (words, starting point) | Job |
| --- | --- | --- | --- | --- |
| AdcTask | 5 | Notification from the DMA half and full transfer interrupts | 256 | Computes mean, minimum and maximum of the finished half-buffer and publishes the latest block |
| RecordTask | 4 | Absolute-time delay every 10 ms (`vTaskDelayUntil`) | 512 | Reads the I²C sensor, takes the latest ADC block, builds one record, updates the shared staging buffer, sends the record to the queue |
| LoadTask (experiment builds only) | 3 | Every 50 ms | 256 | Burns CPU for about 5 ms to create the medium-priority interference needed in section 10 |
| LoggerTask | 2 | Queue receive | 1024 (FatFs needs room) | Collects 17 records into a 512-byte block, adds the CRC, writes to the SD card, reads the shared staging buffer |
| ShellTask | 1 | UART receive | 512 | Parses commands and prints results |
| MonitorTask | 1 | Every 500 ms | 256 | Feeds the hardware watchdog only if every task has set its check-in bit since the last round |

MonitorTask sits at the lowest application priority on purpose: if a higher-priority task starves the system, the monitor never runs, the watchdog is not fed, and the board resets. The builder should be ready to explain this choice.

**Interrupts and timers**

| Resource | Use | Note |
| --- | --- | --- |
| SysTick | FreeRTOS tick (1 kHz) | CubeMX should use a different timer for the HAL time base to avoid conflicts |
| 32-bit timer at 1 MHz | Free-running microsecond timestamp | Handle wrap-around explicitly |
| Timer trigger output | Starts each ADC conversion at the sampling rate | No CPU involvement per sample |
| DMA (ADC) | Circular buffer of 20 samples, half-transfer and full-transfer interrupts | At 1 kHz this gives an event every 10 ms |
| Second timer at 10 to 20 times the tick rate | Run-time statistics clock | Needed for per-task CPU load |
| I²C, SPI, UART | Sensor, SD card, shell | Interrupts that call FreeRTOS `...FromISR` functions must have a numeric priority no more urgent than the configured maximum system-call priority |

**Communication and shared data rules**

- RecordTask to LoggerTask: one queue of record structs (default depth 64). Dropped records are counted, not hidden.
- DMA interrupt to AdcTask: task notification (cheapest signalling primitive; the builder explains why not a queue).
- AdcTask to RecordTask: a single latest-block slot protected by the staging lock (section 10).
- Every shared variable has one owner task written in the module header; anything else is accessed through a queue, a notification or a lock.
- No dynamic allocation after startup; all sizes come from one configuration header.

**Record and block format**

Each SD block is 512 bytes, so a power cut never splits a block across sectors.

| Offset (bytes) | Size | Field |
| --- | --- | --- |
| 0 | 4 | Magic number |
| 4 | 1 | Format version |
| 5 | 1 | Flags |
| 6 | 2 | Record count in this block (17 when full) |
| 8 | 4 | Sequence number of the first record |
| 12 | 4 | Session identifier (increases at every start of logging) |
| 16 | 476 | 17 records of 28 bytes each |
| 492 | 16 | Zero padding |
| 508 | 4 | CRC-32 over bytes 0 to 507 |

A record is 28 bytes: sequence (4), timestamp in microseconds (4), ADC mean, minimum and maximum (2 each), six sensor axes (2 each), and a 2-byte flags field. At 100 records per second this is about 6 blocks per second, roughly 3 kB/s, far below what an SD card sustains.

**Power-loss design (to be validated by the tests in section 11)**

1. Pre-allocate each log file at its full size so power loss never leaves the file size out of date (check how `f_expand` behaves in the FatFs version used).
2. At boot, scan blocks from the start; the end of the log is the first block whose magic, session identifier, sequence continuity or CRC does not match.
3. The session identifier stops old data left on the card from earlier sessions being mistaken for new records.
4. Fallback if FatFs becomes a time sink: write blocks directly to a reserved raw sector range and read them with a Python script that opens the card device; the record format stays identical.

## 10. The priority inversion experiment and fault handling

These two parts turn a working logger into a project with a story. Both are designed to be shown live in an interview.

**Priority inversion experiment**

Priority inversion happens when a high-priority task waits for a resource held by a low-priority task, and a medium-priority task preempts the low-priority one, so the high-priority task is delayed by work that is less important than itself.

| Role | Task | Priority | What it does in the experiment |
| --- | --- | --- | --- |
| High | RecordTask | 4 | Needs the staging lock briefly every 10 ms |
| Medium | LoadTask | 3 | Wakes every 50 ms and runs for about 5 ms without blocking |
| Low | LoggerTask | 2 | Takes the staging lock and keeps it for a few milliseconds while copying and writing a block (an artificial busy delay may be added inside the critical section and must be labeled as artificial in the report) |

1. **Mode A, the problem.** The lock is a plain binary semaphore with no priority inheritance. Sequence: the logger takes the lock; the high-priority task wakes and blocks on it; the medium task becomes ready and preempts the logger; the high-priority task now waits for the logger's remaining time plus the medium task's full burst.
2. **Mode B, priority inheritance.** The lock is a FreeRTOS mutex. While the high-priority task waits, the logger temporarily runs at the high priority, so the medium task cannot preempt it, and the delay shrinks to the logger's critical section.
3. **Mode C, the better design.** Shorten the critical section so the logger holds the lock only for a quick copy and does the slow SD write outside it. This shows that inheritance reduces the damage but good design avoids it.

Measurement: debug GPIO pins show "RecordTask waiting for the lock", "LoggerTask holds the lock" and "LoadTask running" on the logic analyzer. The firmware also records the maximum and mean wait time of RecordTask and counts periods that exceed the 10 ms deadline. The builder reports a before and after table for modes A, B and C, with the logic-analyzer screenshots committed to the repo. The expected qualitative result is that A shows deadline misses, B removes the medium-task delay, and C reduces the wait further; the numbers must be measured, not assumed.

**HardFault capture**

- Replace the default CubeMX handler (an endless loop) with a small assembly stub that selects the stack that was in use (main or process stack) and passes its pointer to a C function.
- The C function copies R0 to R3, R12, LR, PC and xPSR from the stacked frame, plus the fault status registers (CFSR, HFSR, MMFAR, BFAR), into a RAM region excluded from startup zeroing, adds a magic value and a CRC, and triggers a system reset.
- Enable the separate bus, memory-management and usage fault exceptions, and the divide-by-zero trap, so faults are reported precisely instead of all escalating to HardFault.
- At boot, if the region is valid, print the saved values, write a flagged record to the log, and clear the region.
- Decode the saved PC with the toolchain's address-to-line tool (`arm-none-eabi-addr2line`) against the ELF file to find the source line.

**Fault test cases** (triggered by `fault <type>` in the shell, builds with the shell enabled)

| Test | How it is provoked | Expected report |
| --- | --- | --- |
| Invalid memory write | Write to an address in a reserved region | Bus fault with a valid address in BFAR |
| Divide by zero | Integer divide with the trap enabled | Usage fault, DIVBYZERO bit set |
| Unaligned access | Load a word from an odd address with the trap enabled | Usage fault, UNALIGNED bit set |
| Invalid state | Jump through a function pointer with the Thumb bit clear | Usage fault, INVSTATE bit set |
| Stack overflow | Recursive function in a test task | Stack-overflow hook names the task |

**Watchdog and reset cause**

- The monitor task keeps one check-in bit per task (an event group). If one is missing at the end of a round, the monitor writes the mask of missing tasks to the same no-init region and deliberately stops feeding the independent watchdog.
- After the reset, boot code reads the reset-cause flags in the reset and clock control status register (power-on, pin, software, independent watchdog, brown-out), prints them, clears them, and includes the saved missing-task mask when the cause is the watchdog.

## 11. Testing, measurement and validation

Embedded projects are judged by evidence. Each row below produces an artifact (a log, a screenshot, a table) that goes into the repo.

| Area | What is tested | Method and tooling | Pass criteria |
| --- | --- | --- | --- |
| Host-side unit tests | Record packing, CRC-32, block builder, parser, shell command parser, ring-buffer logic | Pure C modules compiled and run on the PC with a small test framework (Unity or similar) | All pass; modules have no hardware dependencies so they can run on the host |
| Log parser | Reads blocks, checks CRC, session and sequence continuity, writes CSV, reports gaps | Python script plus tests with deliberately corrupted files | Every injected corruption is reported with the block number |
| Signal accuracy | ADC path with a known input | Onboard DAC loopback or a function generator at a few frequencies and amplitudes | Recorded mean, minimum and maximum match the input within the ADC error budget (stated in the report) |
| Timing | Sensor period jitter, DMA event period, latency from interrupt to queue | Logic analyzer on the debug pins, 10,000 periods, statistics from the exported capture | Meets NFR-01 and NFR-02, or the gap is explained |
| Endurance | 1-hour and overnight runs at default rates | Automated check with the parser | Zero lost samples; stack and heap margins stay within NFR-06 |
| Slow storage | Logger delayed on purpose (artificial delay or a slow card) | Compare sampling jitter with and without the delay | Sampling unaffected; drops counted correctly |
| Power loss | Abrupt power removal during logging | Switch-based power pull at random moments, 50 repetitions; after each, boot, read the recovery message, run the parser | Every log is parseable; recovered count matches; only the final partial block is lost |
| Watchdog | Hung task, starved system | `wdtest` and a deliberately high-priority busy loop | Reset within 2.5 s; missing-task mask reported |
| Fault handling | Test cases in section 10 | `fault <type>` command | Each is reported correctly and traced to the right source line |
| Priority inversion | Modes A, B, C | Logic analyzer plus firmware wait-time counters | Results table with measured numbers and screenshots |
| Shell | Every command, bad input, overlong lines, rapid input | pyserial test script | No crash, no timing impact (FR-022) |
| Static and build checks | Warnings, static analysis, size | `-Wall -Wextra`, cppcheck, `arm-none-eabi-size` in CI | Clean or justified, size recorded per commit |

**Measurement protocol**

1. Record the setup: board, core clock, compiler and optimisation level, card model, firmware commit hash.
2. Run each measurement at least five times; report median and spread, not the best run.
3. Keep the raw files (logic analyzer captures, CSVs) in the repo so a reader can re-check the numbers.
4. For each missed target, write one paragraph on the cause and what was tried.

**Continuous integration**

A GitHub Actions workflow builds the firmware with the ARM GCC toolchain, runs the host-side unit tests, runs the static analysis, and posts the binary size. Hardware tests are run by hand and documented.

## 12. Milestones

&#91;embedded content: 4-week plan · 4 gates, final gate highlighted\]

Each gate is pass or fail. If a gate fails, the next week starts by fixing it, and the cut order in section 5 applies.

**Deliverables by week**

- **Week 1:** order parts; install the toolchain; three register-level exercises (GPIO, UART, timer interrupt) and an explanation of reset to `main()`; host-side record format, CRC-32 and parser started; CubeMX project with FreeRTOS, `printf` over UART, timer-triggered ADC with DMA and a sequence-numbered sample counter. Gate 1: the counter matches the expected rate for 10 minutes with no lost samples.
- **Week 2:** I²C sensor task with absolute-time delays; record builder; queue; LoggerTask with FatFs (or the raw-sector fallback), 512-byte blocks and CRC; Python parser with gap detection; first logic-analyzer jitter capture. Gate 2: a 1-hour run produces a log the parser reads with every sample present.
- **Week 3:** UART shell with the core commands; monitor task and hardware watchdog; reset-cause report; HardFault capture and the five fault tests; runtime statistics and stack margins; an overnight endurance run. Gate 3: a hung task and an induced fault are both caught and reported correctly after reboot.
- **Week 4:** priority inversion modes A, B and C with measurements; the 50-pull power-loss campaign; final jitter and CPU load tables; README, design notes and the demo video. Gate 4: every success criterion in section 2 is met.

## 13. Risks and mitigations

| # | Risk | Likelihood | Impact | Mitigation |
| --- | --- | --- | --- | --- |
| 1 | The builder is new to ARM Cortex-M and gets stuck on the toolchain, startup code or interrupt priorities | High | High | The learning ramp in section 5; use CubeMX and HAL for setup; write only one peripheral at register level; step through startup in the debugger; ask for help on the ST community forum early rather than after two days |
| 2 | SD card and FatFs integration eats the schedule (wiring, SPI speed, card compatibility) | High | Medium | Time-box to two days; try a second card; fall back to raw sector logging with the same record format |
| 3 | Interrupt priority mistakes cause random crashes or missed notifications | Medium | High | Document the priority scheme on one page; enable FreeRTOS assertions; test with the debugger and the runtime statistics |
| 4 | Stack sizes are guessed and tasks overflow silently | Medium | High | Stack-overflow checking on; report high-water marks; test under the worst case |
| 5 | The priority inversion does not appear because the timing is too forgiving | Medium | Medium | Lengthen the artificial critical section and the medium-priority burst until it shows; state clearly in the report that the delays are artificial |
| 6 | Logic analyzer clone is unreliable or the software setup fails | Medium | Medium | Test it in week 1; fall back to an oscilloscope in the college lab with the same GPIO toggles |
| 7 | Power-loss test damages the SD card or filesystem | Low | Medium | Use a spare card; keep the host-side parser and a second card ready; log the test date and card model |
| 8 | Relying on AI-written firmware the builder cannot explain | Medium | High | AI may explain concepts and review; the builder writes the task code, ISR handlers and the fault handler and records design notes in her own words |
| 9 | Scope creep (wireless, GUI, cloud) | Medium | Medium | Non-goals in section 2 are binding; extra ideas go to the stretch list |
| 10 | Board or parts delayed | Low | High | Order in the first two days; start with simulation-free host-side code (parser, CRC, block builder) while waiting |

## 14. Repo, README, demo and resume packaging

**Repository layout**

```text
stm32-rtos-logger/
  README.md
  docs/            design notes, pin map, architecture diagram, results
  firmware/
    Core/          CubeMX-generated startup and peripheral setup
    app/
      tasks/       adc_task, record_task, logger_task, shell_task, monitor_task, load_task
      drivers/     sensor (I2C), storage (FatFs or raw SD), debug_gpio
      fault/       hardfault handler, noinit region, reset cause
      common/      record format, crc32, config.h
  host/
    tests/         unit tests that run on the PC
    parser/        Python log parser and CSV converter
    tools/         pyserial test scripts
  measurements/    logic analyzer captures, CSVs, screenshots
  .github/workflows/ci.yml
```

**README must contain**

- What it is and what it is not, with a photo of the wired board.
- Architecture diagram, task table, and the shared-data rules.
- Build, flash and run instructions that work from a clean checkout.
- Results table: jitter, CPU load per task, stack margins, loss over the endurance run, the 50-pull power test, and the priority inversion modes A, B and C.
- Fault handling demo: a screenshot of the boot report and the decoded source line.
- Known limitations and what would be done next.
- A "What I learned and what I would change" section.

**Design notes (the interview script).** One page per decision, in the builder's own words: why these priorities, why notifications and not queues for the DMA event, why the monitor task has low priority, why 512-byte blocks with a CRC, why pre-allocate the file, why mode C is better than mode B.

**Demo video (about 3 minutes).** Start the logger and show the `status` output; show the logic analyzer with the sampling period; run the priority inversion in mode A, B and C; trigger a fault and show the decoded report; pull the power and show the recovery message and a clean parse.

**Resume bullets (fill with real measured numbers only)**

- Built a multi-task data logger on an STM32 (Cortex-M4) with FreeRTOS: timer-triggered ADC with DMA, I²C sensor, CRC-protected SD logging and a UART shell; sampling jitter \[X\]% measured on a logic analyzer; CPU load \[Y\]% at \[Z\] Hz.
- Diagnosed and fixed a deliberately induced priority inversion with a priority-inheritance mutex and a shorter critical section; reduced the high-priority task's worst-case wait from \[A\] to \[B\].
- Implemented HardFault capture, a task-checked watchdog and power-loss recovery; verified clean recovery over \[N\] power pulls with zero corrupted logs.

**Interview questions the builder must be able to answer**

1. What happens from reset to `main()` on a Cortex-M?
2. Why do FreeRTOS and the NVIC use opposite priority directions, and what goes wrong if an interrupt is too urgent?
3. Why use DMA for the ADC, and what do the half-transfer and full-transfer interrupts do?
4. Explain priority inversion with your own measurements. Why is inheritance a mitigation and not a cure?
5. How does your logger survive a power cut, and how do you know?
6. What does the saved PC in a HardFault tell you, and how did you map it to a line of code?
7. Why is the monitor task at the lowest priority?
8. What would you change to log at 10 times the rate?

## 15. Assumptions, open questions and first actions

**Assumptions** (change any that are wrong and the plan adjusts)

- A1: The builder has experience with Embedded C and ESP32 or Arduino boards, but little with ARM Cortex-M, STM32 tooling or an RTOS.
- A2: About 15 to 20 hours per week for four weeks, starting on a Monday.
- A3: A Windows or Linux laptop that can run STM32CubeIDE, plus access to a college lab for an oscilloscope if the logic analyzer fails.
- A4: The target is entry-level embedded and hardware-related roles, so measured evidence matters more than feature count.
- A5: All targets in sections 2 and 7 are hypotheses to be replaced by measured values.
- A6: Parts are bought new; the total stays within a student budget.

**Open questions**

- Q1: Which STM32 board will actually be used? Pick one that is available quickly; the plan works with any Cortex-M4 Nucleo board, with the DAC loopback available only on boards that have a DAC.
- Q2: SD card with FatFs, or a raw SPI flash chip? Default: SD card with FatFs, with the raw sector fallback in section 9.
- Q3: Does the college lab lend a logic analyzer or oscilloscope?
- Q4: Should the project also include a small Python plotting tool for the logs? Default: only as a stretch.
- Q5: Is a second sensor desired for the interview story, or is one enough? Default: one.

**First five actions**

1. Order the board, sensor, SD module and logic analyzer on day 1; install CubeIDE and flash the example blink project on day 1 or 2.
2. Do the three register-level exercises (GPIO, UART, timer interrupt) and explain reset to `main()` aloud (days 1 to 3).
3. Build the host-side modules while parts arrive: record format, CRC-32, block builder and the Python parser, with unit tests.
4. Bring up FreeRTOS with two tasks and `printf` over UART, then timer-triggered ADC with DMA and a sample counter (days 4 to 7).
5. Pass Gate 1: the sample counter matches the expected rate for 10 minutes with no lost samples.
