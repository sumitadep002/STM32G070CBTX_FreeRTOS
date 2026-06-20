# STM32G070CBTX FreeRTOS Exploration

This project is dedicated to exploring **FreeRTOS** implementations on the **STM32G070** microcontroller. It focuses on building a robust, asynchronous architecture for managing peripherals like LoRa modems, I2C LCDs, and user inputs via GPIO interrupts.

## Project Objectives
- Study and implement FreeRTOS primitives (Tasks, Queues, Thread Flags).
- Develop event-driven drivers for **SX126x LoRa** modems.
- Implement an ACK-based wireless validation protocol.
- Explore asynchronous LCD management for real-time diagnostics.
- Implement runtime hardware abstraction for multi-variant peripheral support.

## Completed Milestones & Issues

Detailed tracking of project progress can be found in the [Closed Issues](https://github.com/sumitadep002/STM32G070CBTX_FreeRTOS/issues?q=is%3Aissue%20state%3Aclosed) section.

| Issue ID | Description |
| :--- | :--- |
| **#23** | Integrate 2 LCD Modules into One Unified Driver |
| **#21** | Lora TX/RX Link Test & Validation |
| **#19** | Integrate Lora Peripheral Driver Code |
| **#17** | Bring up SPI Communication Interface |
| **#16** | Explore Basics of LoRa Physical Layer |
| **#13** | Implement Asynchronous FreeRTOS-based LCD Management |
| **#12** | Handle User Inputs via Push Button |
| **#9** | Bring up 16x2 I2C LCD |
| **#8** | Bring up I2C Communication Interface |
| **#6** | Process User Input States |
| **#5** | Add GPIO External Interrupt Service Routine (ISR) |
| **#3** | Bring up FreeRTOS Real-Time Kernel |
| **#1** | Implement RTOS-Safe Thread Logging Port |

## LoRa Ping-Pong Protocol
The project features a stable, interrupt-driven ping-pong protocol designed for link validation.
- **Asynchronous Flow**: Uses `osThreadFlags` to synchronize radio interrupts with application tasks.
- **Selective IRQ Handling**: Optimized HAL to process and clear interrupts individually, preventing race conditions.
- **Manual Trigger**: TX mode requires a >1000ms button press to start, allowing for coordinated testing.

### LCD Diagnostic Interface
The 16x2 display provides a high-density, real-time link status for easy troubleshooting.

![LCD Diagnostic Screenshot](image.png)

**Display Legend:**
- **Line 1**: `R:<rx_count> T:<tx_count> <last_payload>`
  - `R`: Total packets received.
  - `T`: Total packets transmitted.
  - `last_payload`: Raw numeric sequence of the last successful exchange.
- **Line 2**: `RSSI:<value> S:<value>`
  - `RSSI`: Signal strength in dBm.
  - `S`: Signal-to-Noise Ratio (SNR) in dB.

---

## LCD Hardware Support
The driver automatically detects and supports two hardware variants of the 16x2 $I^2C$ LCD at startup:
* [cite_start]**Native $I^2C$ Display (Address `0x3E`):** Uses the native register interface (**RG1602A-19-I2C**) for direct block streaming[cite: 12].
* [cite_start]**Backpack $I^2C$ Display (Address `0x27`):** Uses an emulated 4-bit parallel sequence over an integrated **PCF8574T** expander (**RG1602A-I2C(P) Ver1.3**)[cite: 12]. A hardware timer (**TIM1**, prescaled to 63) handles the required microsecond signal pacing.

---

## Development Tools

### LCD Simulator
A Python-based **16x2 LCD Simulator** is included to test and visualize display layouts without physical hardware. It accurately mimics the character grid and wiring of a standard I2C LCD module.

**Prerequisites:**
Ensure you have the Python Tkinter library installed:
```bash
sudo apt-get install python3-tk

```

**Usage:**
Run the simulator from the project root:

```bash
python3 lcd_sim.py

```

## Project Structure

* `project/`: Main STM32CubeIDE project directory.
* `Core/`: Standard initialization (`main.c` incorporating **TIM1** configuration), and main application logic.
* `lora/`: Event-driven SX126x driver and HAL abstraction.
* `lcd/`: Asynchronous, runtime-adaptive I2C LCD driver and FreeRTOS management task.
* `cfg_btn/`: Interrupt-driven button handler with duration measurement.


* `lcd_sim.py`: Python/Tkinter based 16x2 LCD simulator.

## Hardware Requirements

* **Microcontroller**: STM32G070CBTX
* **Hardware Timers**: TIM1 (Configured at 1MHz tick rate for precise microsecond delays)
* **LoRa Modem**: Semtech SX1262 (via SPI)
* **Supported Displays**:
* Variant 1: Native $I^2C$ LCD (**RG1602A-19-I2C** at Address `0x3E`)
* Variant 2: Integrated PCF8574T Backpack LCD (**RG1602A-I2C(P)** at Address `0x27`)


* **Input**: Push button connected to Config Switch Pin (`CFG_SW_Pin`)

## Getting Started

1. **Configure Mode**: Set `LORA_BOARD_MODE` in `lora.h` to `LORA_MODE_TX` or `LORA_MODE_RX`.
2. **Start Receiver**: Power the RX board first; it will automatically enter a listening state.
3. **Start Transmitter**: Power the TX board. It will display `TX READY`.
4. **Trigger Test**: Hold the User Button on the TX board for **1000ms** to start the 100-packet sequence.