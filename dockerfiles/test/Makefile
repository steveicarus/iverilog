CC = iverilog
PROG = hello
PROG_TB = $(PROG)_tb

$(PROG_TB).vcd: $(PROG_TB).vvp
	vvp $(PROG_TB).vvp

$(PROG_TB).vvp: $(PROG).v $(PROG_TB).v
	$(CC) -o $(PROG_TB).vvp $(PROG_TB).v

clean:
	rm $(PROG_TB).vvp $(PROG_TB).vcd
	
 
