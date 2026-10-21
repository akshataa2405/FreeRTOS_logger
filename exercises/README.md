# Cortex-M4 Bare-Metal Learning Ramp Exercises

These exercises are designed to be inspected, stepped through in the debugger, and explained aloud without notes during embedded firmware interviews.

---

## 1. What happens from Reset to `main()` on ARM Cortex-M?

1. **Hardware Power-on / Reset deassertion:**
   - The CPU fetches the initial **Main Stack Pointer (MSP)** value from memory location `0x00000000` (aliased to Flash at `0x08000000`).
   - The CPU fetches the **Reset Handler address** from `0x00000004` (second entry in the vector table).
   - Bit 0 of the Reset Handler address is 1 (Thumb bit indicator); execution begins in Thumb mode.
2. **`Reset_Handler` Execution:**
   - **SystemInit:** Configures FPU (CP10/CP11 full access in `SCB->CPACR`), resets RCC clocks.
   - **Data Initialization:** Copies the `.data` section from Flash (LMA) to SRAM (VMA).
   - **BSS Zeroing:** Clears the `.bss` section in SRAM to `0`.
   - **Static Constructors:** Calls `__libc_init_array` if C++ or static constructors are used.
   - **Branch to Application:** Jumps to `main()`.

---

## 2. Why do FreeRTOS and NVIC use Opposite Priority Directions?

- **ARM Cortex-M NVIC Hardware:**
  - **Lower numerical value = HIGHER urgency / priority.**
  - Priority 0 is the highest priority interrupt, while Priority 15 (on a 4-bit priority MCU like STM32F4) is the lowest interrupt priority.
  - Furthermore, Cortex-M implements only the most significant bits of each 8-bit priority register (e.g. bits [7:4] in STM32F4).
- **FreeRTOS Software Scheduler:**
  - **Higher numerical value = HIGHER urgency / priority.**
  - Priority 0 is the `IdleTask`, Priority 5 is higher than Priority 1.
- **The Critical Trap:**
  - Any ISR calling FreeRTOS API functions (`xQueueSendFromISR`, `xTaskNotifyFromISR`) **MUST** have an NVIC priority numerically greater than or equal to `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` (i.e. logically equal or lower priority).
  - If an interrupt with a higher hardware priority (lower number, e.g. 0 to 4) calls a FreeRTOS API, it can interrupt FreeRTOS critical sections, causing silent data corruption and erratic HardFaults!

---

## 3. Summary of Register Exercises

| Exercise | File | Key Peripherals & Registers |
|---|---|---|
| **GPIO Bare-Metal** | [`gpio_bare.c`](./gpio_bare.c) | `RCC_AHB1ENR`, `GPIOA_MODER`, `GPIOA_OTYPER`, `GPIOA_BSRR` |
| **UART Bare-Metal** | [`uart_bare.c`](./uart_bare.c) | `RCC_APB1ENR`, `GPIOA_AFRL` (AF7), `USART2_BRR`, `USART2_CR1`, `USART2_SR`, `USART2_DR` |
| **Timer Interrupt** | [`tim_irq.c`](./tim_irq.c) | `TIM2_PSC`, `TIM2_ARR`, `TIM2_DIER`, `TIM2_SR`, `NVIC_ISER0`, `NVIC_IPR7` |
