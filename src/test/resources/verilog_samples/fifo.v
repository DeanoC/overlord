// Synchronous FIFO implementation
// Common design pattern found in many GitHub repos

module sync_fifo #(
    parameter DATA_WIDTH = 8,
    parameter DEPTH = 16,
    parameter ALMOST_FULL_THRESHOLD = 12,
    parameter ALMOST_EMPTY_THRESHOLD = 4
)(
    input wire clk,
    input wire rst_n,
    
    // Write interface
    input wire wr_en,
    input wire [DATA_WIDTH-1:0] wr_data,
    output wire full,
    output wire almost_full,
    
    // Read interface
    input wire rd_en,
    output reg [DATA_WIDTH-1:0] rd_data,
    output wire empty,
    output wire almost_empty,
    
    // Status
    output reg [$clog2(DEPTH):0] fill_level
);

    // Local parameters
    localparam ADDR_WIDTH = $clog2(DEPTH);
    
    // Memory array for FIFO data
    reg [DATA_WIDTH-1:0] mem [0:DEPTH-1];
    
    // Pointers
    reg [ADDR_WIDTH:0] wr_ptr;
    reg [ADDR_WIDTH:0] rd_ptr;
    
    // Full/empty conditions
    assign empty = (wr_ptr == rd_ptr);
    assign full = (wr_ptr[ADDR_WIDTH-1:0] == rd_ptr[ADDR_WIDTH-1:0]) && 
                  (wr_ptr[ADDR_WIDTH] != rd_ptr[ADDR_WIDTH]);
    
    // Almost full/empty flags
    assign almost_empty = (fill_level <= ALMOST_EMPTY_THRESHOLD);
    assign almost_full = (fill_level >= ALMOST_FULL_THRESHOLD);
    
    // Implementation details omitted for testing purposes

endmodule
