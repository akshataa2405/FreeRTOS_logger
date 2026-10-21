# System Architecture

## 1. Multi-Task Firmware Pipeline

```mermaid
flowchart TD
    subgraph Hardware Peripherals
        TIM2[Timer 2 @ 1 kHz TRGO] -->|Hardware Trigger| ADC1[ADC1 + DMA Circular Double Buffer]
        MPU[MPU6050 6-DOF IMU]
        SD[MicroSD Card SPI / FatFs]
        IWDG[Independent Watchdog ~2s]
    end

    subgraph FreeRTOS Tasks
        ADC1 -->|Half/Full DMA ISR Notification| T1[AdcTask - Priority 5]
        T1 -->|Protected Staging Slot| T2[RecordTask - Priority 4]
        MPU -->|I2C Burst Read| T2
        T2 -->|Static Queue 64 Depth| T3[LoggerTask - Priority 2]
        T3 -->|512-Byte CRC-32 Blocks| SD

        T4[LoadTask - Priority 3<br>Experiment Only] -.->|Burns 5ms CPU| CPU[CPU Preemption]

        T5[ShellTask - Priority 1] -->|UART Virtual COM Port| UART[Interactive Terminal]

        T1 & T2 & T3 & T5 -->|EventGroup Check-in Bits| T6[MonitorTask - Priority 1]
        T6 -->|Refresh on all healthy| IWDG
    end
```

---

## 2. Shared Data and Concurrency Rules

1. **AdcTask to RecordTask:**
   - Single slot `g_latest_adc` updated every 10 ms.
   - Protected by `g_staging_lock` (Mode A: Semaphore, Mode B/C: Mutex).
2. **RecordTask to LoggerTask:**
   - FreeRTOS static queue `g_record_queue` of depth 64 holding `DataRecord_t` structs (28 bytes each).
   - If queue is saturated (e.g., slow SD write), RecordTask drops newest sample and increments `s_dropped_samples`.
   - Logging is **strictly non-blocking** to high-priority sensor sampling.
3. **MonitorTask Check-in Scheme:**
   - Every active task sets its bit in `s_monitor_events`.
   - MonitorTask runs at lowest application priority (Priority 1). If any high-priority task starves the CPU, MonitorTask never runs -> watchdog resets the MCU.
