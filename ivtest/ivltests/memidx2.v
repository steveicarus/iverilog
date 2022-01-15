// Memory test for index bug
module main ();
reg [7:0] mem [0:64];
reg [7:0] val_reg;
wire [7:0] val_wire;

// Works ok
assign val_wire = mem[67];

initial
  begin

    // This should generate an error.
    val_reg = mem;

  end


endmodule
