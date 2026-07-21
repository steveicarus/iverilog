// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`ifdef OVL_ASSERT_ON

  parameter REQ_ACK_START    = 2'b00;
  parameter REQ_ACK_WAIT     = 2'b01;
  parameter REQ_ACK_ERR      = 2'b10;
  parameter REQ_ACK_DEASSERT = 2'b11;

  reg [1:0] r_state;
  reg [1:0] r_r_state;
  reg r_req;
  reg r_ack;
  integer i;
  integer j;

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  wire valid_req;
  wire valid_ack;

  assign valid_req = ~( req ^ req );
  assign valid_ack = ~( ack ^ ack );
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

`ifdef OVL_SYNTHESIS
`else
  initial begin
    r_state=REQ_ACK_START;
    r_r_state=REQ_ACK_START;
    r_req=0;
    r_ack=0;
    i = 0;
    j = 0;
  end
`endif

  always @(posedge clk) begin
      if (`OVL_RESET_SIGNAL != 1'b0) begin // active low reset
        case (r_state)
          REQ_ACK_START:
            begin
              if ((max_ack_length != 0) && ack == 1'b1 && r_ack == 1'b1)
                begin
                  j <= j+1;
                  if (j >= max_ack_length)
                    begin
                      r_state <= REQ_ACK_ERR;
                      ovl_error_t(`OVL_FIRE_2STATE,"Duration of continuous asserted state of acknowledge violates specified maximum ack_max_length cycles");
                    end
                end
              else if ((max_ack_length != 0) && ack == 1'b0 )
                begin
                  j <= 0;
                end
              else if ((max_ack_length != 0) && ack == 1'b1 && r_ack == 1'b0)
                begin
                  j <= 1;
                end

              if (req == 1'b1) begin
                if (r_ack == 1'b1 && ack == 1'b1 && r_req == 1'b0) begin
                  r_state <= REQ_ACK_ERR;
                  ovl_error_t(`OVL_FIRE_2STATE,"New request arrives before previous request is acknowledged");
                end
                else if (deassert_count != 0 && r_req == 1'b1 && req == 1'b1 &&
                    r_ack == 1'b1) begin
                  if (deassert_count == 1'b1) begin
                    r_state <= REQ_ACK_ERR;
                    ovl_error_t(`OVL_FIRE_2STATE,"Duration of continuous asserted state of request violates specified deassert_count cycles");
                  end
                  else begin
                    r_state <= REQ_ACK_DEASSERT;
                    i <= deassert_count-1;
                  end
                end
                else if (ack == 1'b1 && r_ack == 1'b0 && r_req == 1'b1)
                  begin
                    r_state <= REQ_ACK_ERR;
                    ovl_error_t(`OVL_FIRE_2STATE,"Acknowledge arrives without a pending request");
                  end
                else if ((min_ack_cycle != 0) && ack && r_ack == 1'b0)
                  begin
                    r_state <= REQ_ACK_ERR;
                    ovl_error_t(`OVL_FIRE_2STATE,"Acknowledge asserted before elapse of specified minimum min_ack_cycle cycles from request");
                  end
                else if (ack == 1'b0 && r_req === 1'b0) begin
                  r_state <= REQ_ACK_WAIT;
                  i <= 1;
                  j <= 0;
                end
              end
              else begin
                if (ack == 1'b1 && r_ack == 1'b0) begin
                  r_state <= REQ_ACK_ERR;
                  ovl_error_t(`OVL_FIRE_2STATE,"Acknowledge arrives without a pending request");
                end
              end
            end
          REQ_ACK_WAIT:
            begin
              i <= i + 1;
              if (ack) begin
                r_state <= REQ_ACK_START;
                j <= 1;
              end

              if ((min_ack_cycle != 0) && (i < min_ack_cycle) &&
			ack == 1'b1) begin
                r_state <= REQ_ACK_ERR;
                ovl_error_t(`OVL_FIRE_2STATE,"Acknowledge asserted before elapse of specified minimum min_ack_cycle cycles from request");
              end
              else if ((!ack) && (max_ack_cycle != 0) && i >= max_ack_cycle) begin

                r_state <= REQ_ACK_ERR;
                ovl_error_t(`OVL_FIRE_2STATE,"Acknowledge is not asserted within specified maximum max_ack_cycle cycles from request");
              end
              else if (req_drop == 1'b1 && req == 1'b0) begin
                r_state <= REQ_ACK_ERR;
                ovl_error_t(`OVL_FIRE_2STATE,"Request is deasserted before acknowledgement arrives");
              end
              else if (req == 1'b1 && r_req == 1'b0) begin
                r_state <= REQ_ACK_ERR;
                ovl_error_t(`OVL_FIRE_2STATE,"New request arrives before previous request is acknowledged");
              end
            end
          REQ_ACK_ERR:
            begin
              if (req == 1'b1 && ack == 1'b0 && r_req == 1'b0) begin
                r_state <= REQ_ACK_WAIT;
                i <= 1;
                j <= 0;
              end
              else if (req == 1'b1 && r_req == 1'b0 && ack == 1'b1 && r_ack == 1'b0)
                begin
                  if (min_ack_cycle != 0)
                    begin
                      ovl_error_t(`OVL_FIRE_2STATE,"Acknowledge asserted before elapse of specified minimum min_ack_cycle cycles from request");
                    end
                  else
                    begin
                      r_state <= REQ_ACK_START;
                      i <= 1;
                      j <= 1;
                    end
                end
              else if (ack == 1'b0) begin
                r_state <= REQ_ACK_START;
                i <= 0;
                j <= 0;
              end
            end
          REQ_ACK_DEASSERT:
            begin
              i <= i-1;

              if( req == 1'b0 )
                begin
                  r_state <= REQ_ACK_START;
                  i <= 0;
                end
              else if((req == 1'b1) && (i == 1))
                begin
                  r_state <= REQ_ACK_ERR;
                  ovl_error_t(`OVL_FIRE_2STATE,"Duration of continuous asserted state of request violates specified deassert_count cycles");
                end

              if ((max_ack_length != 0) && ack == 1'b1 && r_ack == 1'b1)
                begin
                  j <= j+1;
                  if (j >= max_ack_length)
                    begin
                      r_state <= REQ_ACK_ERR;
                      ovl_error_t(`OVL_FIRE_2STATE,"Duration of continuous asserted state of acknowledge violates specified maximum ack_max_length cycles");
                    end
                end
              else if ((max_ack_length != 0) && ack == 1'b0 )
                begin
                  j <= 0;
                end
              else if ((max_ack_length != 0) && ack == 1'b1 && r_ack == 1'b0)
                begin
                  r_state <= REQ_ACK_ERR;
                  ovl_error_t(`OVL_FIRE_2STATE,"Acknowledge arrives without a pending request");
                end
            end
        endcase
        r_r_state <= r_state;
        r_ack <= ack;
        r_req <= req;

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else

        if ( valid_req == 1'b1 )
          begin
            //Do nothing
          end
        else
          begin
            ovl_error_t(`OVL_FIRE_XCHECK,"req contains X or Z");
          end

        if ( valid_ack == 1'b1 )
          begin
            //Do nothing
          end
        else
          begin
            ovl_error_t(`OVL_FIRE_XCHECK,"ack contains X or Z");
          end

 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

      end
      else begin
         r_state <= REQ_ACK_START;
         r_r_state <= REQ_ACK_START;
         r_ack <= 0;
         r_req <= 0;
         i <= 0;
         j <= 0;
      end

  end // always

`endif // OVL_ASSERT_ON

`ifdef OVL_COVER_ON

  reg prev_req, prev_ack;

  always @(posedge clk) begin
   if (`OVL_RESET_SIGNAL != 1'b0) begin
    if (coverage_level != `OVL_COVER_NONE) begin

     if (OVL_COVER_BASIC_ON) begin //basic coverage
        prev_req <= req;
        prev_ack <= ack;
        if (prev_req != req && prev_req == 1'b0) begin
          ovl_cover_t("req_asserted covered");
        end
        if (prev_ack != ack && prev_ack == 1'b0) begin
          ovl_cover_t("ack_asserted covered");
        end
     end //basic coverage

    end // OVL_COVER_NONE
   end
   else begin
`ifdef OVL_INIT_REG
     prev_req <= 0;
     prev_ack <= 0;
`endif
   end
  end //always

`endif // OVL_COVER_ON

