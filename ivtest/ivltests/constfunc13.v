// Test mixed type/width case expressions inside a constant function
module constfunc13();

function [2:0] lookup1(input signed [2:0] value);

begin
  case (value)
    4'sb0100 : lookup1 = 1;
    3'sb100  : lookup1 = 2;
    2'sb10   : lookup1 = 3;
    default  : lookup1 = 4;
  endcase
  $display("case = %d", lookup1);
end

endfunction

function [2:0] lookup2(input signed [2:0] value);

begin
  case (value)
    4'b1100 : lookup2 = 1;
    3'sb100 : lookup2 = 2;
    2'sb10  : lookup2 = 3;
    default : lookup2 = 4;
  endcase
  $display("case = %d", lookup2);
end

endfunction

function [2:0] lookup3(input real value);

begin
  case (value)
    4'b0001 : lookup3 = 1;
    3'sb010 : lookup3 = 2;
    2'sb11  : lookup3 = 3;
    default : lookup3 = 4;
  endcase
  $display("case = %d", lookup3);
end

endfunction

function [2:0] lookup4(input signed [2:0] value);

begin
  case (value)
    4'b0110 : lookup4 = 1;
    3'sb110 : lookup4 = 2;
    -1.0    : lookup4 = 3;
    default : lookup4 = 4;
  endcase
  $display("case = %d", lookup4);
end

endfunction

localparam res11 = lookup1(3'sb100);
localparam res12 = lookup1(3'sb110);
localparam res13 = lookup1(3'sb010);

localparam res21 = lookup2(3'sb100);
localparam res22 = lookup2(3'sb010);
localparam res23 = lookup2(3'sb110);

localparam res31 = lookup3( 1.0);
localparam res32 = lookup3( 2.0);
localparam res33 = lookup3(-1.0);
localparam res34 = lookup3( 1.5);

localparam res41 = lookup4(3'sb110);
localparam res42 = lookup4(3'sb111);
localparam res43 = lookup4(3'sb011);

reg failed = 0;

initial begin
  $display("case = %d", res11); if (res11 != 2) failed = 1;
  $display("case = %d", res12); if (res12 != 3) failed = 1;
  $display("case = %d", res13); if (res13 != 4) failed = 1;
  $display("");
  $display("case = %d", res21); if (res21 != 2) failed = 1;
  $display("case = %d", res22); if (res22 != 3) failed = 1;
  $display("case = %d", res23); if (res23 != 4) failed = 1;
  $display("");
  $display("case = %d", res31); if (res31 != 1) failed = 1;
  $display("case = %d", res32); if (res32 != 2) failed = 1;
  $display("case = %d", res33); if (res33 != 3) failed = 1;
  $display("case = %d", res34); if (res34 != 4) failed = 1;
  $display("");
  $display("case = %d", res41); if (res41 != 2) failed = 1;
  $display("case = %d", res42); if (res42 != 3) failed = 1;
  $display("case = %d", res43); if (res43 != 4) failed = 1;
  $display("");
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
