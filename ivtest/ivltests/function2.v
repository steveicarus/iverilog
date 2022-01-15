/*
 * This program handles the case of a system task within a user
 * defined function.
 */
module main;

        reg     [31:0]  tmp1;
        reg     [31:0]  tmp2;

        function [31:0] test;
                input   [31:0]  op1;

                $write("op1 = %h\n", op1);

        endfunction

        initial
        begin
                tmp1 = 'hdeadbeef;
                tmp2 = test(tmp1);
                $display("PASSED");
        end

endmodule
