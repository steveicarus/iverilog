/*
 * Based on Request id 1313366 in the iverilog Bugs database, or
 * pr699 in the ivl-bugs database.
 * Modified to force the comparison net.
 */
module bug;

wire a, b, c, d;

assign c = 1'b0;
assign a = 1'b1;
assign b = 1'b0;

assign d = c ? a : b;

initial
  begin
    force c = 1'b1;
    #1 if (c !== 1'b1) begin
       $display("FAILED -- b = %b", b);
       $finish;
    end

    if (d !== 1'b1) begin
       $display("FAILED -- d = %b", d);
       $finish;
    end

    release c;
    $display("PASSED");
    $finish;
  end

endmodule // bug
