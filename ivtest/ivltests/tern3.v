module test;

    reg [0:0] stat;
    initial begin
        stat = 1'b0;
        // This should display (Start).
        $display("(%s)", stat[0] ? "Stop" : "Start");

        // This should also display (Start). It's been known
        // to display (tart) by getting the expression width
        // from the true clause.
        $display("(%s)", !stat[0] ? "Start" : "Stop");

        $display("$bits == %0d", $bits(stat[0] ? "Stop" : "Start"));
        if ($bits(stat[0] ? "Stop" : "Start") !== 40) begin
	   $display("FAILED");
	   $finish;
	end

        $display("$bits == %0d", $bits(stat[0] ? "Start" : "Stop"));
        if ($bits(stat[0] ? "Start" : "Stop") !== 40) begin
	   $display("FAILED");
	   $finish;
	end
    end

endmodule
