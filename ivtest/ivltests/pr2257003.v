module inner ();
    initial
        a.dump;
endmodule

module outer ();
    inner i ();

    generate
    begin : a
        task dump;
            begin
                $display ("PASSED");
            end
        endtask
    end
    endgenerate
endmodule
