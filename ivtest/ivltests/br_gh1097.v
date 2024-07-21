module test_mod ();

    typedef enum logic [4:0] {ENUM_ELEM1, ENUM_ELEM2} test_enum_t;

    test_enum_t test_mem_addr_e;
    logic [1:0] test_mem [test_mem_addr_e.num()];

    initial begin
        test_mem[ENUM_ELEM1] = 1;
        test_mem[ENUM_ELEM2] = 2;
        $display("ENUM_ELEM1 = %d test_mem[ENUM_ELEM1] = %d", ENUM_ELEM1, test_mem[ENUM_ELEM1]);
        $display("ENUM_ELEM2 = %d test_mem[ENUM_ELEM2] = %d", ENUM_ELEM2, test_mem[ENUM_ELEM2]);
        if (test_mem[ENUM_ELEM1] === 1 && test_mem[ENUM_ELEM2] === 2)
            $display("PASSED");
        else
            $display("FAILED");
    end
endmodule
