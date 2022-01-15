/*
 * This trivial example tickled PR508, where the thread
 * pointer in the event was uninitialized. This should
 * in general check the case of signaling events without
 * threads ever having waiting on it.
 */
module example;
    event          my_event;
    initial -> my_event;
    reg     [25:0] m26;
    wire    [25:0] w26 = m26;
    initial $display("PASSED");
endmodule
