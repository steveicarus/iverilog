#ifndef IVL_uparr_ops_H
#define IVL_uparr_ops_H
/*
 * Operation codes for the common %uparr/reduce opcode.
 */
enum uparr_reduce_op_e {
      UPARR_REDUCE_SUM = 0,
      UPARR_REDUCE_PRODUCT = 1,
      UPARR_REDUCE_AND = 2,
      UPARR_REDUCE_OR = 3,
      UPARR_REDUCE_XOR = 4
};
#endif
