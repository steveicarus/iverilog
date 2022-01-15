/*
 * Based on PR#941.
 * This tests that trivial contant expressions passed as input to
 * user defined tasks will work. A possible bug would be that the
 * addition expression gets useless code generated.
 */
module test;

task foo;
input [16:0] in1;
begin
   $display("%d", in1);
   $display("PASSED");
end
endtask

initial begin
         foo(16'h00 + 'h00);
end

endmodule
