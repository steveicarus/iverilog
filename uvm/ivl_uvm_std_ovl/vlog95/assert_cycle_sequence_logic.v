// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

  parameter NC0 = (necessary_condition == `OVL_TRIGGER_ON_MOST_PIPE);
  parameter NC1 = (necessary_condition == `OVL_TRIGGER_ON_FIRST_PIPE);
  parameter NC2 = (necessary_condition == `OVL_TRIGGER_ON_FIRST_NOPIPE);

  // Guarded parameters for num_cks < 2 (which is bad usage - see warning in top-level file)
  parameter NUM_CKS_1 = (num_cks > 0) ? (num_cks - 1) : 0;
  parameter NUM_CKS_2 = (num_cks > 1) ? (num_cks - 2) : 0;
  parameter LSB_1     = (num_cks > 1) ?            1  : 0;


//------------------------------------------------------------------------------
// SHARED CODE
//------------------------------------------------------------------------------
`ifdef OVL_SHARED_CODE
  reg [NUM_CKS_1:0] seq_queue; // REVISIT: bit [0] is redundant (Mantis #1812)
  always @(posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      seq_queue <= {num_cks{1'b0}};
    end
    else begin
      seq_queue[NUM_CKS_2:0] <= seq_queue[NUM_CKS_1:1] & event_sequence[NUM_CKS_2:0];
      seq_queue[NUM_CKS_1]   <= NC2 ? event_sequence[NUM_CKS_1] && (~(|seq_queue[NUM_CKS_1:1]))
                                    : event_sequence[NUM_CKS_1];
    end
  end
`endif // OVL_SHARED_CODE


//------------------------------------------------------------------------------
// ASSERTION
//------------------------------------------------------------------------------
`ifdef OVL_ASSERT_ON

  // 2-STATE
  // =======
  wire fire_2state_1, fire_2state_2;
  always @(posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      // OVL does not fire during reset
    end
    else begin
      if (fire_2state_1) begin
        ovl_error_t(`OVL_FIRE_2STATE,"First event occured but it is not followed by the rest of the events in sequence");
      end
      if (fire_2state_2) begin
        ovl_error_t(`OVL_FIRE_2STATE,"First num_cks-1 events occured but they are not followed by the last event in sequence");
      end
    end
  end

  assign fire_2state_1 = ((NC1 || NC2) && (&((seq_queue[NUM_CKS_1:LSB_1] & event_sequence[NUM_CKS_2:0]) | ~(seq_queue[NUM_CKS_1:LSB_1])) == 1'b0));
  assign fire_2state_2 = ( NC0         && ((!seq_queue[1] || ((seq_queue[1] && event_sequence[0]))) == 1'b0));


  // X-CHECK
  // =======
  `ifdef OVL_XCHECK_OFF
  `else
    `ifdef OVL_IMPLICIT_XCHECK_OFF
    `else
      reg fire_xcheck_1, fire_xcheck_2, fire_xcheck_3, fire_xcheck_4, fire_xcheck_5, fire_xcheck_6;
      always @(posedge clk) begin
        if (`OVL_RESET_SIGNAL == 1'b0) begin
          // OVL does not fire during reset
        end
        else begin
          if (fire_xcheck_1) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"First event in the sequence contains X or Z");
          end
          if (fire_xcheck_2) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"First event in the sequence contains X or Z");
          end
          if (fire_xcheck_3) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"Subsequent events in the sequence contain X or Z");
          end
          if (fire_xcheck_4) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"First num_cks-1 events in the sequence contain X or Z");
          end
          if (fire_xcheck_5) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"Last event in the sequence contains X or Z");
          end
          if (fire_xcheck_6) begin
            ovl_error_t(`OVL_FIRE_XCHECK,"First num_cks-1 events in the sequence contain X or Z");
          end
        end
      end

      wire valid_first_event = ~( event_sequence[NUM_CKS_1] ^
                                  event_sequence[NUM_CKS_1] );

      wire valid_sequence = (~((^(seq_queue[NUM_CKS_1:LSB_1] &
                                event_sequence[NUM_CKS_2:0] &
                             {{(NUM_CKS_2){1'b1}},{~(|NC0)}})) ^
                             (^(seq_queue[NUM_CKS_1:LSB_1] &
                                event_sequence[NUM_CKS_2:0] &
                             {{(NUM_CKS_2){1'b1}},{~(|NC0)}}))));

      wire valid_last_event = ~((seq_queue[1] && event_sequence[0]) ^
                                (seq_queue[1] && event_sequence[0]));

      always @ (valid_first_event or seq_queue) begin
        if (valid_first_event) begin
          fire_xcheck_1 = 1'b0;
          fire_xcheck_2 = 1'b0;
        end
        else begin
          fire_xcheck_1 = (NC0 || NC1);
          fire_xcheck_2 = (NC2 && ~(|seq_queue[NUM_CKS_1:1]));
        end
      end

      always @ (valid_sequence) begin
        if (valid_sequence) begin
          fire_xcheck_3 = 1'b0;
          fire_xcheck_4 = 1'b0;
        end
        else begin
          fire_xcheck_3 = (NC1 || NC2);
          fire_xcheck_4 = (NC0);
        end
      end

      always @ (valid_last_event or seq_queue) begin
        if (valid_last_event) begin
          fire_xcheck_5 = 1'b0;
          fire_xcheck_6 = 1'b0;
        end
        else begin
          fire_xcheck_5 = (NC0 &&  seq_queue[1]);
          fire_xcheck_6 = (NC0 && ~seq_queue[1]);
        end
      end

    `endif // OVL_IMPLICIT_XCHECK_OFF
  `endif // OVL_XCHECK_OFF

`endif // OVL_ASSERT_ON


//------------------------------------------------------------------------------
// COVERAGE
//------------------------------------------------------------------------------
`ifdef OVL_COVER_ON

  wire fire_cover_1, fire_cover_2;
  always @ (posedge clk) begin
    if (`OVL_RESET_SIGNAL == 1'b0) begin
      // OVL does not fire during reset
    end
    else begin
      if (fire_cover_1) begin
        ovl_cover_t("sequence_trigger covered"); // basic
      end
      if (fire_cover_2) begin
        ovl_cover_t("sequence_trigger covered"); // basic
      end
    end
  end

  assign fire_cover_1 = ((OVL_COVER_BASIC_ON > 0) && (NC1 || NC2) && event_sequence[NUM_CKS_1]);
  assign fire_cover_2 = ((OVL_COVER_BASIC_ON > 0) &&  NC0         && (&seq_queue[1])); // REVISIT: Reduction-AND is redundant

`endif // OVL_COVER_ON
