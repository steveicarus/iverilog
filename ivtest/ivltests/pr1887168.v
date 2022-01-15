// Copyright 2008, Martin Whitaker.
// This file may be freely copied for any purpose.

module constant_integer_div_mod();

initial begin
  $display("%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d",
            31'd10 / 'd3,
            31'd11 / 'd3,
            31'd12 / 'd3,
            31'sd10 /  3,
            31'sd11 /  3,
            31'sd12 /  3,
           -31'sd10 /  3,
           -31'sd11 /  3,
           -31'sd12 /  3,
            31'sd10 / -3,
            31'sd11 / -3,
            31'sd12 / -3,
           -31'sd10 / -3,
           -31'sd11 / -3,
           -31'sd12 / -3);

  $display("%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d",
            65'd10 / 'd3,
            65'd11 / 'd3,
            65'd12 / 'd3,
            65'sd10 /  3,
            65'sd11 /  3,
            65'sd12 /  3,
           -65'sd10 /  3,
           -65'sd11 /  3,
           -65'sd12 /  3,
            65'sd10 / -3,
            65'sd11 / -3,
            65'sd12 / -3,
           -65'sd10 / -3,
           -65'sd11 / -3,
           -65'sd12 / -3);

  $display("%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d",
            31'd10 % 'd3,
            31'd11 % 'd3,
            31'd12 % 'd3,
            31'sd10 %  3,
            31'sd11 %  3,
            31'sd12 %  3,
           -31'sd10 %  3,
           -31'sd11 %  3,
           -31'sd12 %  3,
            31'sd10 % -3,
            31'sd11 % -3,
            31'sd12 % -3,
           -31'sd10 % -3,
           -31'sd11 % -3,
           -31'sd12 % -3);

  $display("%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d",
            65'd10 % 'd3,
            65'd11 % 'd3,
            65'd12 % 'd3,
            65'sd10 %  3,
            65'sd11 %  3,
            65'sd12 %  3,
           -65'sd10 %  3,
           -65'sd11 %  3,
           -65'sd12 %  3,
            65'sd10 % -3,
            65'sd11 % -3,
            65'sd12 % -3,
           -65'sd10 % -3,
           -65'sd11 % -3,
           -65'sd12 % -3);
end

endmodule
