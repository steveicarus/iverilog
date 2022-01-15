`define DEV_TYPE  "DEVICE 2"

module top;
    parameter device = `DEV_TYPE;
    wire res;

function is_dev1;
    input[8*20:1] device;
    reg is_device;
begin
    if ((device == "DEVICE1") || (device == "DEVICE 1"))
        is_device = 1;
    else
        is_device = 0;

    is_dev1  = is_device;
end
endfunction

function is_dev2;
    input[8*20:1] device;
    reg is_device;
begin
    if ((device == "DEVICE2") || (device == "DEVICE 2"))
        is_device = 1;
    else
        is_device = 0;

    is_dev2  = is_device;
end
endfunction

function is_dev;
    input[8*20:1] device;
    reg is_device;
begin
// Changing this to a single item makes things work.
    if (is_dev1(device) || is_dev2(device))
        is_device = 1;
    else
        is_device = 0;

    is_dev  = is_device;
end
endfunction

    assign res = (is_dev(device) == 1) ? 1'b1 : 1'b0;

    initial #1 if (res == 1'b1) $display("PASSED");
               else $display("FAILED");
endmodule
