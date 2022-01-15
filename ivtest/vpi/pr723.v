module test;
    integer i;
    reg [7:0] c;
    reg [31:0] fd[15:0];

    initial begin
        // Open some MCDs and FD's to play with
        $display("Open some files");
        for (i = 0; i < 4; i = i+1) begin
            fd[i] = $fopen("/dev/null");
            $display("open MCD      returned %x", fd[i]);
        end
        for (i = 4; i < 6; i = i+1) begin
            fd[i] = $fopen("/dev/null", "r");
            $display("open FD ('r') returned %x", fd[i]);
        end
        for (i = 6; i < 8; i = i+1) begin
            fd[i] = $fopen("/dev/null", "w");
            $display("open FD ('w') returned %x", fd[i]);
        end
        for (i = 8; i < 10; i = i+1) begin
            fd[i] = $fopen("/dev/null", "a");
            $display("open FD ('a') returned %x", fd[i]);
        end

        // Show what's open
        $test;
        // Different systems have different buffer sizes so flush here.
        $fflush;

        // access some of the files
        $fdisplay(1, "write to MCD 1");
        $fdisplay(2, "write to MCD 2");
        $fflush;
        $fdisplay(32'h8000_0000, "write to FD 0"); // stdin so invisible
        $fdisplay(32'h8000_0001, "write to FD 1");
        $fdisplay(32'h8000_0002, "write to FD 2"); // stderr is normally unbuffered
                                                   // so will appear before stdout

        // close all the stuff we opened
        $display("Close some files");
        for (i = 0; i < 10; i = i+1) begin
            $fclose(fd[i]);
        end

        // try closing special and already closed
        $fclose(1);
        $fclose(2);
        $fclose(32'h4000_0000);

        $fclose(32'h8000_0000);
        $fclose(32'h8000_0001);
        $fclose(32'h8000_0002);
        $fclose(32'h8000_0003);

        // Now for something really insane
        $fclose(32'h81ca_1ca0);

        // Show what's open
        $test;

        // access to null MCD/FD's
        $fdisplay(32'h4000_0000, "write to NULL MCD");
        $fdisplay(32'h8000_000f, "write to NULL FD");
    end
endmodule
