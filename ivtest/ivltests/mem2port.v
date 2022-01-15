/*
 * Bug report:
 *
 * From: Hendrik <hendrik@humanistic.org>
 * Subject: gEDA: Pass array element into module in iverilog 0.5
 * To: geda-dev@seul.org
 * Date: Mon, 10 Sep 2001 11:53:04 +0800
 */

module top;
        reg [6:0] x[2:0];

        speak i0 (x[0], x[1], x[2]);

        initial
        begin
                #10     x[0] = 0;
                        x[1] = 0;
                        x[2] = 0;
                #100    x[0] = 1;
                #100    x[0] = 0;
                        x[1] = 1;
                #100    x[1] = 0;
                        x[2] = 1;
                #100    $finish;
        end
endmodule

module speak(x1, x2, x3);
        input [6:0] x1, x2, x3;
        always #100
                $display ("%d: x1=%d, x2=%d, x3=%d", $time, x1, x2, x3);

   integer	    errors;
   initial
     begin
	errors = 0;
	#100 if (x1 !== 7'b0 || x2 !== 7'b0 || x3 !== 7'b0)
	  begin errors = errors + 1; $display("FAILED"); end
	#100 if (x1 !== 7'b1 || x2 !== 7'b0 || x3 !== 7'b0)
	  begin errors = errors + 1; $display("FAILED"); end
	#100 if (x1 !== 7'b0 || x2 !== 7'b1 || x3 !== 7'b0)
	  begin errors = errors + 1; $display("FAILED"); end
	#100 if (x1 !== 7'b0 || x2 !== 7'b0 || x3 !== 7'b1)
	  begin errors = errors + 1; $display("FAILED"); end
	if (errors === 0)
	  $display("PASSED");
     end

endmodule
