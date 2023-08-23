// Check that it is possible to have a `output reg` in a UDP definition

module test;

  reg clk = 1'b0;
  reg d = 1'b0;
  wire q;

  dff ff(q, clk, d);

  initial begin
    #1
    clk = 1'b1;
    #1
    clk = 1'b0;
    d = 1'b1;

    if (q === 1'b0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule

primitive dff(q, c, d);
  output reg q;
  input c, d;
  table
  //c d : q : q+
    p 0 : ? : 0 ;
    p 1 : ? : 1 ;
    n ? : ? : - ;
    ? * : ? : - ;
  endtable
endprimitive
