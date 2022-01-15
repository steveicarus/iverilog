module top;
  localparam wid = 7;
  localparam vec_wid = $clog2(wid+1)-1;
  reg pass;
  reg [vec_wid:0] mem [wid:0];
  reg [wid:0] sel;
  wor [vec_wid:0] out;
  integer lp;
  genvar i;

  for (i = 0; i <= wid; i = i + 1) assign out = sel[i] ? mem[i] : {wid{1'b0}};

  initial begin
    pass = 1'b1;

    for (lp = 0; lp <= wid; lp = lp + 1) begin
      mem[lp] = lp;
    end

    for (lp = 0; lp <= wid; lp = lp + 1) begin
      sel = 2**lp;
      #1;
      if (out !==  mem[lp]) begin
        $display("FAILED: mem[%0d] %b != %b (%b)", lp, mem[lp], out, sel);
        pass = 1'b0;
      end else $display("OK: mem[%0d] %b (%b)", lp, out, sel);
    end

    if (pass) $display("PASSED");
  end
endmodule
