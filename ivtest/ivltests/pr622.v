`define FOO bar
module foo;
         initial begin
                 $display("macro FOO = %s", ``FOO);
         end
endmodule
