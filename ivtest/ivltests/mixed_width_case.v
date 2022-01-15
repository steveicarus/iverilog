module mixed_width_case();

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

reg [2:0] result;

reg failed = 0;

initial begin
  result = lookup1(3'sb100); if ( result != 2) failed = 1;
  result = lookup1(3'sb110); if ( result != 3) failed = 1;
  result = lookup1(3'sb010); if ( result != 4) failed = 1;
  $display("");
  result = lookup2(3'sb100); if ( result != 2) failed = 1;
  result = lookup2(3'sb010); if ( result != 3) failed = 1;
  result = lookup2(3'sb110); if ( result != 4) failed = 1;
  $display("");
  result = lookup3( 1.0); if ( result != 1) failed = 1;
  result = lookup3( 2.0); if ( result != 2) failed = 1;
  result = lookup3(-1.0); if ( result != 3) failed = 1;
  result = lookup3( 1.5); if ( result != 4) failed = 1;
  $display("");
  result = lookup4(3'sb110); if ( result != 2) failed = 1;
  result = lookup4(3'sb111); if ( result != 3) failed = 1;
  result = lookup4(3'sb011); if ( result != 4) failed = 1;
  $display("");
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
