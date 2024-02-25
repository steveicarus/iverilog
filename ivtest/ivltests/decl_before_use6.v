module test();

localparam w = 8;

task t;
  reg [w:1] v;
  localparam w = 2;
  begin
    v = 8'hAA;
    $display("%b", v);
    if (v === 8'hAA)
      $display("PASSED");
    else
      $display("FAILED");
  end
endtask;

initial t;

endmodule
