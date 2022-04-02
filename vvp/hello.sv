module main();
    // reg int foo;
    reg clk;
    reg rst;

    reg [31:0] cpp_in;
    reg [31:0] cpp_out;
    reg [31:0] cpp_mem_wr_req;
    reg [31:0] cpp_mem_wr_addr;
    reg [31:0] cpp_mem_wr_data;

    reg [31:0] n_cpp_in;

    reg [31:0] mem [16];

    task tick();
        #5 clk = 0;
        #5 clk = 1;
    endtask

    always @(*) begin
        n_cpp_in = 0;
    end

    always @(posedge clk, posedge rst) begin
        if(rst) begin
            cpp_in <= 0;
        end else begin
            cpp_in <= n_cpp_in;
        end
        if(cpp_mem_wr_req) begin
            mem[cpp_mem_wr_addr] = cpp_mem_wr_data;
        end
    end

    initial begin
        rst = 1;
        tick();

        rst <= 0;
        tick();

        tick();
        for(int i = 0; i < 4; i++) begin
            $cpp_memread(i, cpp_mem_wr_req, cpp_mem_wr_addr, cpp_mem_wr_data);
            tick();
        end
        tick();
        tick();
        tick();
        tick();
        tick();
        tick();

        for(int i = 0; i < 16; i++) begin
            $display("i %0d %0d", i, mem[i]);
        end
    end
endmodule
