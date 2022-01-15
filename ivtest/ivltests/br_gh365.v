module tb();

typedef enum logic [1:0] {
    IDLE    = 0,
    RESET   = 1,
    START   = 2,
    WAITFOR = 3
} stateType;

stateType state;

string workingString = "WORKING";

initial begin
    $display("DIRECT ASSIGNED STRING is ", workingString);
    #10;
    state = IDLE;
end

string state_txt;

always @* begin
    case(state)
        IDLE    : state_txt = "IDLE";
        RESET   : state_txt = "RST";
        START   : state_txt = "STRT";
        WAITFOR : state_txt = "WAIT";
    endcase
    $display("Controller's new state is %s",state_txt);
end

endmodule
