// Test for GitHub issue #670
// Class method can have the same name as the class
program main;
    class test;
        int value;

        // Method with same name as class - should be valid
        function void test();
            $display("test method called");
            value = 42;
        endfunction
    endclass

    test tst;

    initial begin
        tst = new();
        tst.test();
        if (tst.value !== 42) begin
            $display("FAILED: value = %0d, expected 42", tst.value);
            $finish;
        end
        $display("PASSED");
    end
endprogram
