# Verilog UART Module Design Plan

## Overview
This document outlines the design of a reusable Verilog UART module with the following features:
- Variable input clock support for multi-board compatibility
- Support for common baud rates up to a few megabaud
- 8N1 format (8 data bits, no parity, 1 stop bit)
- Memory-mapped I/O (MMIO) interface for CPU interaction
- Parameterized FIFOs for TX and RX buffers

## Module Architecture

### Top-Level Structure
```
                           +---------------------+
                           |                     |
                           |   UART Controller   |
                           |                     |
                           +---------------------+
                           |                     |
            +--------------|     MMIO Logic      |
CPU         |              |                     |
Interface   |              +---------------------+
<----------->              |                     |
            |              | Baud Rate Generator |
            |              |                     |
            |              +---------------------+
            |              |         |           |
            |              |         v           |
            |    +---------+     +-------+       |
            |    |         |     |       |       |
            +--->| TX FIFO +---->| TX    +-----> TX Pin
                 |         |     | Logic |       |
                 +---------+     +-------+       |
                 |         |     |       |       |
                 | RX FIFO <-----+ RX    <------ RX Pin
                 |         |     | Logic |       |
                 +---------+     +-------+       |
                                                 |
```

## Module Parameters

```verilog
module uart_core #(
    parameter CLK_FREQ_HZ = 50_000_000, // Default 50MHz clock
    parameter DEFAULT_BAUD = 115200,    // Default baud rate
    parameter TX_FIFO_DEPTH = 16,       // TX buffer depth
    parameter RX_FIFO_DEPTH = 16,       // RX buffer depth
    parameter DATA_BITS = 8,            // Always 8 for 8N1
    parameter STOP_BITS = 1,            // Always 1 for 8N1
    parameter PARITY = "NONE",          // Always NONE for 8N1
    parameter OVERSAMPLE = 16           // Oversampling factor
) (
    // Clock and reset
    input wire clk,
    input wire rst_n,
    
    // Serial interface
    input wire rx,
    output wire tx,
    
    // MMIO interface
    input wire [3:0] addr,        // Register address
    input wire [31:0] wdata,      // Write data
    output reg [31:0] rdata,      // Read data
    input wire wr_en,             // Write enable
    input wire rd_en,             // Read enable
    
    // Interrupt signals
    output wire tx_empty_irq,     // TX FIFO empty interrupt
    output wire rx_ready_irq,     // RX data ready interrupt
    output wire rx_overrun_irq    // RX overrun error interrupt
);
    // Module implementation will go here
endmodule
```

## Clock Division and Baud Rate Generation

The module will support variable input clocks with a configurable divider to generate the desired baud rate. The baud rate generator will:

1. Use a fractional divider to achieve accurate baud rates from any input clock
2. Support dynamic reconfiguration via the MMIO interface
3. Support standard baud rates: 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600, 1000000, 2000000, 3000000

```verilog
// Baud rate calculation
// Division Factor = (CLK_FREQ_HZ) / (BAUD_RATE * OVERSAMPLE)
```

## FIFO Implementation

Both TX and RX FIFOs will be implemented using parameterized depths:

1. Each FIFO will have configurable depth via module parameters
2. Gray code counters for read/write pointers to handle clock domain crossing
3. Status flags: EMPTY, FULL, HALF_FULL, THRESHOLD
4. Optional FIFO threshold interrupt triggers

## MMIO Register Map

| Offset | Register Name | Description |
|--------|--------------|-------------|
| 0x00   | CTRL         | Control register (enable, reset FIFOs) |
| 0x04   | STATUS       | Status register (FIFO levels, errors) |
| 0x08   | BAUD_DIV     | Baud rate divisor |
| 0x0C   | TX_DATA      | TX FIFO write port |
| 0x10   | RX_DATA      | RX FIFO read port |
| 0x14   | INT_ENABLE   | Interrupt enable bits |
| 0x18   | INT_STATUS   | Interrupt status bits |

### Register Details

#### CTRL Register (0x00)
```
Bit 0: UART Enable
Bit 1: TX FIFO Reset
Bit 2: RX FIFO Reset
Bit 3: Loopback Mode
Bits 7-4: Reserved
Bits 15-8: TX FIFO Threshold
Bits 23-16: RX FIFO Threshold
Bits 31-24: Reserved
```

#### STATUS Register (0x04)
```
Bit 0: TX FIFO Empty
Bit 1: TX FIFO Full
Bit 2: RX FIFO Empty
Bit 3: RX FIFO Full
Bit 4: TX FIFO Threshold Reached
Bit 5: RX FIFO Threshold Reached
Bit 6: RX Frame Error
Bit 7: RX Overrun Error
Bits 15-8: TX FIFO Level
Bits 23-16: RX FIFO Level
Bits 31-24: Reserved
```

## Synchronization Techniques

The UART module will operate in multiple clock domains:
- System clock domain (CPU interface)
- UART clock domain (baud rate)

Synchronization will be implemented using:
1. Dual-flip-flop synchronizers for control signals
2. Gray-coded pointers for FIFO read/write addresses
3. Asynchronous FIFO for data transfer between clock domains

## Error Handling

The UART will detect and report:
1. Framing errors (invalid stop bit)
2. Overflow errors (RX FIFO full when new data arrives)
3. Break conditions (RX line held low for more than one frame)

## Implementation Files

1. `uart_top.v` - Top-level module with MMIO interface
2. `uart_tx.v` - Transmitter logic
3. `uart_rx.v` - Receiver logic
4. `uart_fifo.v` - Parameterized FIFO implementation
5. `uart_baud_gen.v` - Baud rate generator with fractional divider
6. `uart_regs.v` - Register file for MMIO interface

## Testing Strategy

1. Unit tests for each submodule (baud generator, FIFO, TX, RX)
2. Integration tests for the complete UART
3. Loopback tests connecting TX to RX
4. Tests with various clock frequencies and baud rates
5. MMIO interface tests with register reads/writes

## Implementation Phases

1. Core TX/RX logic with fixed baud rate
2. Baud rate generator with variable clock support
3. FIFO implementation for TX and RX
4. MMIO register interface
5. Full integration and testing

## Resource Estimation

For a typical FPGA implementation:
- Logic Elements: ~300-500
- Flip-Flops: ~200-300
- Block RAM: 2-4 blocks (depending on FIFO depths)

## Future Extensions

1. Support for parity modes (even, odd)
2. Support for 2 stop bits
3. DMA interface for high-speed data transfer
4. Flow control (RTS/CTS)