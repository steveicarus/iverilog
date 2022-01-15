// Test case statements inside a constant function
module constfunc12();

function [1:0] onehot_to_binary(input [3:1] x);

case (x)
  default : onehot_to_binary = 0;
  3'b001  : onehot_to_binary = 1;
  3'b010  : onehot_to_binary = 2;
  3'b100  : onehot_to_binary = 3;
endcase

endfunction

function [1:0] find_first_one(input [3:1] x);

casez (x)
  3'b1??  : find_first_one = 3;
  3'b01?  : find_first_one = 2;
  3'b001  : find_first_one = 1;
  default : find_first_one = 0;
endcase

endfunction

function [1:0] find_first_zero(input [3:1] x);

casex (x)
  3'b0zz  : find_first_zero = 3;
  3'b10x  : find_first_zero = 2;
  3'b110  : find_first_zero = 1;
  default : find_first_zero = 0;
endcase

endfunction

function [1:0] match_real_value(input real x);

case (x)
  1.0     : match_real_value = 1;
  2.0     : match_real_value = 2;
  3.0     : match_real_value = 3;
  default : match_real_value = 0;
endcase

endfunction

localparam otb0 = onehot_to_binary(3'b000);
localparam otb1 = onehot_to_binary(3'b001);
localparam otb2 = onehot_to_binary(3'b010);
localparam otb3 = onehot_to_binary(3'b100);
localparam otb4 = onehot_to_binary(3'b101);
localparam otb5 = onehot_to_binary(3'b10z);
localparam otb6 = onehot_to_binary(3'bx01);

localparam ffo0 = find_first_one(3'b000);
localparam ffo1 = find_first_one(3'b001);
localparam ffo2 = find_first_one(3'b01x);
localparam ffo3 = find_first_one(3'b1xx);
localparam ffo4 = find_first_one(3'bxx1);
localparam ffo5 = find_first_one(3'b00z);
localparam ffo6 = find_first_one(3'b0zz);
localparam ffo7 = find_first_one(3'bzzz);

localparam ffz0 = find_first_zero(3'b111);
localparam ffz1 = find_first_zero(3'b110);
localparam ffz2 = find_first_zero(3'b10x);
localparam ffz3 = find_first_zero(3'b0xx);
localparam ffz4 = find_first_zero(3'bzzz);
localparam ffz5 = find_first_zero(3'b11x);
localparam ffz6 = find_first_zero(3'b1xx);
localparam ffz7 = find_first_zero(3'bxxx);

localparam mrv0 = match_real_value(0.0);
localparam mrv1 = match_real_value(1.0);
localparam mrv2 = match_real_value(2.0);
localparam mrv3 = match_real_value(3.0);
localparam mrv4 = match_real_value(4.0);

reg failed = 0;

initial begin
  $display("%d", otb0); if (otb0 !== 2'd0) failed = 1;
  $display("%d", otb1); if (otb1 !== 2'd1) failed = 1;
  $display("%d", otb2); if (otb2 !== 2'd2) failed = 1;
  $display("%d", otb3); if (otb3 !== 2'd3) failed = 1;
  $display("%d", otb4); if (otb4 !== 2'd0) failed = 1;
  $display("%d", otb5); if (otb5 !== 2'd0) failed = 1;
  $display("%d", otb6); if (otb6 !== 2'd0) failed = 1;
  $display("");
  $display("%d", ffo0); if (ffo0 !== 2'd0) failed = 1;
  $display("%d", ffo1); if (ffo1 !== 2'd1) failed = 1;
  $display("%d", ffo2); if (ffo2 !== 2'd2) failed = 1;
  $display("%d", ffo3); if (ffo3 !== 2'd3) failed = 1;
  $display("%d", ffo4); if (ffo4 !== 2'd0) failed = 1;
  $display("%d", ffo5); if (ffo5 !== 2'd1) failed = 1;
  $display("%d", ffo6); if (ffo6 !== 2'd2) failed = 1;
  $display("%d", ffo7); if (ffo7 !== 2'd3) failed = 1;
  $display("");
  $display("%d", ffz0); if (ffz0 !== 2'd0) failed = 1;
  $display("%d", ffz1); if (ffz1 !== 2'd1) failed = 1;
  $display("%d", ffz2); if (ffz2 !== 2'd2) failed = 1;
  $display("%d", ffz3); if (ffz3 !== 2'd3) failed = 1;
  $display("%d", ffz4); if (ffz4 !== 2'd3) failed = 1;
  $display("%d", ffz5); if (ffz5 !== 2'd1) failed = 1;
  $display("%d", ffz6); if (ffz6 !== 2'd2) failed = 1;
  $display("%d", ffz7); if (ffz7 !== 2'd3) failed = 1;
  $display("");
  $display("%d", mrv0); if (mrv0 !== 2'd0) failed = 1;
  $display("%d", mrv1); if (mrv1 !== 2'd1) failed = 1;
  $display("%d", mrv2); if (mrv2 !== 2'd2) failed = 1;
  $display("%d", mrv3); if (mrv3 !== 2'd3) failed = 1;
  $display("%d", mrv4); if (mrv4 !== 2'd0) failed = 1;
  $display("");
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
