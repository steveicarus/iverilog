module test;
reg [1:0] result;
initial begin
    $display("hello world, 'b%b", 1'b1);
    result = get_bytes(4'b0111);

end

function [1:0] get_bytes;
input [3:0] in_byte_enable;
reg my_byte;

begin
    my_byte = 3;
    begin: while_block
        while (in_byte_enable[my_byte] == 1)
        begin
            $display("Byte enable is 'h%h", my_byte);
            if(my_byte == 0)
                disable while_block;
            my_byte = my_byte - 1;
        end
    end
    get_bytes = 2'b11;
end
endfunction
endmodule
