// Check different parts of a variable can be procedurally and continuously assigned.
module test();

logic [11:0] v;

assign v[7:4] = 4'd1;

reg failed = 0;

initial begin
  v[11:8] = 4'd2;
  #0 $display("%b", v);
  if (v !== 12'b00100001xxxx) failed = 1;
/*
 * IEEE 1800-2017 states that "A force or release statement shall not be
 * applied to a variable that is being assigned by a mixture of continuous
 * and procedural assignments.", but some other compilers allow this. It
 * looks to be more work to detect and report it as an error than to allow
 * it.
 */
  force v[7:4] = 8'd3;
  #0 $display("%b", v);
  if (v !== 12'b00100011xxxx) failed = 1;
  force v[11:8] = 8'd4;
  #0 $display("%b", v);
  if (v !== 12'b01000011xxxx) failed = 1;
  release v[7:4];
  #0 $display("%b", v);
  if (v !== 12'b01000001xxxx) failed = 1;
  release v[11:8];
  #0 $display("%b", v);
  if (v !== 12'b01000001xxxx) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
