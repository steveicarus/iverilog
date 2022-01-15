module test ();
    parameter p = 0;

    reg dummy;
    initial dummy = block.f(0);



    generate case(1)
    p==0:
    begin : block
        function f;
            input i;
            begin
                $display("p == 0:  %0s", p==0?"OK":"FAILED");
	       if (! (p==0)) $finish;
            end
        endfunction
    end
    default:
    begin : block
        function f;
            input i;
            begin
                $display("default: %0s", p!=0?"OK":"FAILED");
	       if (p==0) $finish;
            end
        endfunction
    end
    endcase
    endgenerate
endmodule

module top ();
    test #(0) a();
    test #(1) b();
    initial #1 $display("PASSED");
endmodule
