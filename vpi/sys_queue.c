/*
 *  Copyright (C) 2011-2021  Cary R. (cygcary@yahoo.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "sys_priv.h"
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include "ivl_alloc.h"

/*
 * The two queue types.
 */
#define IVL_QUEUE_FIFO 1
#define IVL_QUEUE_LIFO 2

/*
 * The statistical codes that can be passed to $q_exam().
 */
#define IVL_QUEUE_LENGTH 1
#define IVL_QUEUE_MEAN 2
#define IVL_QUEUE_MAX_LENGTH 3
#define IVL_QUEUE_SHORTEST 4
#define IVL_QUEUE_LONGEST 5
#define IVL_QUEUE_AVERAGE 6

/*
 * All the values that can be returned by the queue tasks/function.
 */
#define IVL_QUEUE_OK 0
#define IVL_QUEUE_FULL 1
#define IVL_QUEUE_UNDEFINED_ID 2
#define IVL_QUEUE_EMPTY 3
#define IVL_QUEUE_UNSUPPORTED_TYPE 4
#define IVL_QUEUE_INVALID_LENGTH 5
#define IVL_QUEUE_DUPLICATE_ID 6
#define IVL_QUEUE_OUT_OF_MEMORY 7
/* Icarus specific status codes. */
#define IVL_QUEUE_UNDEFINED_STAT_CODE 8
#define IVL_QUEUE_VALUE_OVERFLOWED 9
#define IVL_QUEUE_NO_STATISTICS 10

/*
 * Routine to add the given time to the the total time (high/low).
 */
static void add_to_wait_time(uint64_t *high, uint64_t *low, uint64_t c_time)
{
      uint64_t carry = 0U;

      if ((UINT64_MAX - *low) < c_time) carry = 1U;
      *low += c_time;
      assert((carry == 0U) || (*high < UINT64_MAX));
      *high += carry;
}

/*
 * Routine to divide the given total time (high/low) by the number of
 * items to get the average.
 */
static uint64_t calc_average_wait_time(uint64_t high, uint64_t low, uint64_t total)
{
      int bit = 64;
      uint64_t result = 0U;
      assert(total != 0U);
      if (high == 0U) return (low/total);

	/* This is true by design, but since we can only return 64 bits
	 * make sure nothing went wrong. */
      assert(high < total);

	/* It's a big value so calculate the average the long way. */
      do {
	    unsigned carry = 0U;
	      /* Copy bits from low to high until we have a bit to place
	       * in the result or there are no bits left. */
	    while ((bit >= 0) && (high < total) && !carry) {
		    /* If the MSB is set then we will have a carry. */
		  if (high > (UINT64_MAX >> 1)) carry = 1U;
		  high <<= 1;
		  high |= (low & 0x8000000000000000) != 0;
		  low <<= 1;
		  bit -= 1;
	    }

	      /* If this is a valid bit, set the appropriate bit in the
	       * result and subtract the total from the current value. */
	    if (bit >= 0) {
		  result |=  UINT64_C(1) << bit;
		  high = high - total;
	    }
	/* Loop until there are no bits left. */
      } while (bit > 0);

      return result;
}

/*
 * The data structure used for an individual queue element. It hold four
 * state result for the jobs and inform fields along with the time that
 * the element was added in base time units.
 */
typedef struct t_ivl_queue_elem {
      uint64_t time;
      s_vpi_vecval job;
      s_vpi_vecval inform;
} s_ivl_queue_elem, *p_ivl_queue_elem;

/*
 * This structure is used to represent a specific queue. The time
 * information is in base simulation units.
 */
typedef struct t_ivl_queue_base {
      uint64_t shortest_wait_time;
      uint64_t first_add_time;
      uint64_t latest_add_time;
      uint64_t wait_time_high;
      uint64_t wait_time_low;
      uint64_t number_of_adds;
      p_ivl_queue_elem queue;
      PLI_INT32 id;
      PLI_INT32 length;
      PLI_INT32 type;
      PLI_INT32 head;
      PLI_INT32 elems;
      PLI_INT32 max_len;
      PLI_INT32 have_shortest_statistic;
} s_ivl_queue_base, *p_ivl_queue_base;

/*
 * For now we keep the queues in a vector since there are likely not too many
 * of them. We may need something more efficient later.
 */
static p_ivl_queue_base base = NULL;
static int64_t base_len = 0;

/*
 * This routine is called at the end of simulation to free the queue memory.
 */
static PLI_INT32 cleanup_queue(p_cb_data cause)
{
      PLI_INT32 idx;
      (void) cause;  /* Unused argument. */
      for (idx = 0; idx < base_len; idx += 1) free(base[idx].queue);
      free(base);
      base = NULL;
      base_len = 0;
      return 0;
}

/*
 * Add a new queue to the list, return 1 if there is not enough memory,
 * otherwise return 0.
 */
static unsigned create_queue(PLI_INT32 id, PLI_INT32 type, PLI_INT32 length)
{
      p_ivl_queue_base new_base;
      p_ivl_queue_elem queue;

	/* Allocate space for the new queue base. */
      base_len += 1;
      new_base = (p_ivl_queue_base) realloc(base,
                                            base_len*sizeof(s_ivl_queue_base));

	/* If we ran out of memory then fix the length and return a fail. */
      if (new_base == NULL) {
	    base_len -= 1;
	    return 1;
      }
      base = new_base;

	/* Allocate space for the queue elements. */
      queue = (p_ivl_queue_elem) malloc(length*sizeof(s_ivl_queue_elem));

	/* If we ran out of memory then fix the length and return a fail. */
      if (queue == NULL) {
	    base_len -= 1;
	    return 1;
      }

	/* The memory was allocated so configure it. */
      base[base_len-1].queue = queue;
      base[base_len-1].id = id;
      base[base_len-1].length = length;
      base[base_len-1].type = type;
      base[base_len-1].head = 0;
      base[base_len-1].elems = 0;
      base[base_len-1].max_len = 0;
      base[base_len-1].shortest_wait_time = UINT64_MAX;
      base[base_len-1].first_add_time = 0U;
      base[base_len-1].latest_add_time = 0U;
      base[base_len-1].wait_time_high = 0U;
      base[base_len-1].wait_time_low = 0U;
      base[base_len-1].number_of_adds = 0U;
      base[base_len-1].have_shortest_statistic = 0;
      return 0;
}

/*
 * Check to see if the given queue is full.
 */
static unsigned is_queue_full(int64_t idx)
{
      if (base[idx].elems >= base[idx].length) return 1;

      return 0;
}

/*
 * Add the job and inform to the queue. Return 1 if the queue is full,
 * otherwise return 0.
 */
static unsigned add_to_queue(int64_t idx, p_vpi_vecval job,
                             p_vpi_vecval inform)
{
      PLI_INT32 length = base[idx].length;
      PLI_INT32 type = base[idx].type;
      PLI_INT32 head = base[idx].head;
      PLI_INT32 elems = base[idx].elems;
      PLI_INT32 loc;
      s_vpi_time cur_time;
      uint64_t c_time;

      assert(elems <= length);

	/* If the queue is full we can't add anything. */
      if (elems == length) return 1;

	/* Increment the number of element since one will be added.*/
      base[idx].elems += 1;

	/* Save the job and inform to the queue. */
      if (type == IVL_QUEUE_LIFO) {
	    assert(head == 0);  /* For a LIFO head must always be zero. */
	    loc = elems;
      } else {
	    assert(type == IVL_QUEUE_FIFO);
	    loc = head + elems;
	    if (loc >= length) loc -= length;
      }
      base[idx].queue[loc].job.aval = job->aval;
      base[idx].queue[loc].job.bval = job->bval;
      base[idx].queue[loc].inform.aval = inform->aval;
      base[idx].queue[loc].inform.bval = inform->bval;

	/* Save the current time with this entry for the statistics. */
      cur_time.type = vpiSimTime;
      vpi_get_time(NULL, &cur_time);
      c_time = cur_time.high;
      c_time <<= 32;
      c_time |= cur_time.low;
      base[idx].queue[loc].time = c_time;

	/* Increment the maximum length if needed. */
      if (base[idx].max_len == elems) base[idx].max_len += 1;

	/* Update the inter-arrival statistics. */
      assert(base[idx].number_of_adds < UINT64_MAX);
      base[idx].number_of_adds += 1;
      if (base[idx].number_of_adds == 1) base[idx].first_add_time = c_time;
      base[idx].latest_add_time = c_time;

      return 0;
}

/*
 * Get the job and inform values from the queue. Return 1 if the queue is
 * empty, otherwise return 0.
 */
static unsigned remove_from_queue(int64_t idx, p_vpi_vecval job,
                                  p_vpi_vecval inform)
{
      PLI_INT32 type = base[idx].type;
      PLI_INT32 head = base[idx].head;
      PLI_INT32 elems = base[idx].elems - 1;
      PLI_INT32 loc;
      s_vpi_time cur_time;
      uint64_t c_time;

      assert(elems >= -1);

	/* If the queue is empty we can't remove anything. */
      if (elems < 0) return 1;

	/* Decrement the number of element in the queue structure since one
	 * will be removed.*/
      base[idx].elems -= 1;

	/* Remove the job and inform from the queue. */
      if (type == IVL_QUEUE_LIFO) {
	    assert(head == 0);  /* For a LIFO head must always be zero. */
	    loc = elems;
      } else {
	    assert(type == IVL_QUEUE_FIFO);
	    loc = head;
	    if (head + 1 == base[idx].length) base[idx].head = 0;
	    else base[idx].head += 1;
      }
      job->aval = base[idx].queue[loc].job.aval;
      job->bval = base[idx].queue[loc].job.bval;
      inform->aval = base[idx].queue[loc].inform.aval;
      inform->bval = base[idx].queue[loc].inform.bval;

	/* Get the current simulation time. */
      cur_time.type = vpiSimTime;
      vpi_get_time(NULL, &cur_time);
      c_time = cur_time.high;
      c_time <<= 32;
      c_time |= cur_time.low;

	/* Set the shortest wait time if needed. */
      assert(c_time >= base[idx].queue[loc].time);
      c_time -= base[idx].queue[loc].time;
      if (c_time < base[idx].shortest_wait_time) {
	    base[idx].shortest_wait_time = c_time;
      }
      base[idx].have_shortest_statistic = 1;

	/* Add the current element wait time to the total wait time. */
      add_to_wait_time(&(base[idx].wait_time_high), &(base[idx].wait_time_low),
                       c_time);

      return 0;
}

/*
 * Return the current queue length.
 */
static PLI_INT32 get_current_queue_length(int64_t idx)
{
      return base[idx].elems;
}

/*
 * Return the maximum queue length.
 */
static PLI_INT32 get_maximum_queue_length(int64_t idx)
{
      return base[idx].max_len;
}

/*
 * Return the longest wait time in the queue in base simulation units.
 * Make sure to check that there are elements in the queue before calling
 * this routine. The caller will need to scale the time as appropriate.
 */
static uint64_t get_longest_queue_time(int64_t idx)
{
      s_vpi_time cur_time;
      uint64_t c_time;

	/* Get the current simulation time. */
      cur_time.type = vpiSimTime;
      vpi_get_time(NULL, &cur_time);
      c_time = cur_time.high;
      c_time <<= 32;
      c_time |= cur_time.low;

	/* Subtract the element with the longest time (the head) from the
	 * current time. */
      assert(c_time >= base[idx].queue[base[idx].head].time);
      c_time -= base[idx].queue[base[idx].head].time;

      return c_time;
}

/*
 * Check to see if there are inter-arrival time statistics.
 */
static unsigned have_interarrival_statistic(int64_t idx)
{
      return (base[idx].number_of_adds >= 2U);
}

/*
 * Return the mean inter-arrival time for the queue. This is just the
 * latest add time minus the first add time divided be the number of time
 * deltas (the number of adds - 1).
 */
static uint64_t get_mean_interarrival_time(int64_t idx)
{
      return ((base[idx].latest_add_time - base[idx].first_add_time) /
               (base[idx].number_of_adds - 1U));
}

/*
 * Check to see if there are shortest wait time statistics.
 */
static unsigned have_shortest_wait_statistic(int64_t idx)
{
      return (base[idx].have_shortest_statistic != 0);
}

/*
 * Return the shortest amount of time an element has waited in the queue.
 */
static uint64_t get_shortest_wait_time(int64_t idx)
{
      return base[idx].shortest_wait_time;
}

/*
 * Check to see if we have an average wait time statistics.
 */
static unsigned have_average_wait_statistic(int64_t idx)
{
      return (base[idx].number_of_adds >= 1U);
}

/*
 * Return the average wait time in the queue.
 */
static uint64_t get_average_wait_time(int64_t idx)
{
      PLI_INT32 length = base[idx].length;
      PLI_INT32 loc = base[idx].head;
      PLI_INT32 elems = base[idx].elems;
      PLI_INT32 count;
	/* Initialize the high and low time with the current total time. */
      uint64_t high = base[idx].wait_time_high;
      uint64_t low = base[idx].wait_time_low;
      s_vpi_time cur_time;
      uint64_t c_time;

	/* Get the current simulation time. */
      cur_time.type = vpiSimTime;
      vpi_get_time(NULL, &cur_time);
      c_time = cur_time.high;
      c_time <<= 32;
      c_time |= cur_time.low;

	/* For each element still in the queue, add its wait time to the
	 * total wait time. */
      for (count = 0; count < elems; count += 1) {
	    uint64_t add_time = base[idx].queue[loc].time;
	    assert(c_time >= add_time);
	    add_to_wait_time(&high, &low, c_time-add_time);

	      /* Move to the next element. */
	    loc += 1;
	    if (loc == length) loc = 0;
      }

	/* Return the average wait time. */
      return calc_average_wait_time(high, low, base[idx].number_of_adds);
}

/*
 * Check to see if the given id already exists. Return the index for the
 * queue if it exists, otherwise return -1.
 */
static int64_t get_id_index(PLI_INT32 id)
{
      int64_t idx;

      for (idx = 0; idx < base_len; idx += 1) {
	    if (id == base[idx].id) return idx;
      }

      return -1;
}

/*
 * Check to see if the given value is bit based and has 32 or fewer bits.
 */
static unsigned is_32_or_smaller_obj(vpiHandle obj)
{
      PLI_INT32 const_type;
      unsigned rtn = 0;

      assert(obj);

      switch(vpi_get(vpiType, obj)) {
	case vpiConstant:
	case vpiParameter:
	    const_type = vpi_get(vpiConstType, obj);
	    if ((const_type != vpiRealConst) &&
	        (const_type != vpiStringConst)) rtn = 1;
	    break;

	  /* These can have valid 32 bit or smaller numeric values. */
	case vpiIntegerVar:
	case vpiBitVar:
	case vpiByteVar:
	case vpiShortIntVar:
	case vpiIntVar:
	case vpiMemoryWord:
	case vpiNet:
	case vpiPartSelect:
	case vpiReg:
	    rtn = 1;
	    break;
      }

	/* The object must be 32 bits or smaller. */
      if (vpi_get(vpiSize, obj) > 32) rtn = 0;

      return rtn;
}

/*
 * Check to see if the argument is a variable that is exactly 32 bits in size.
 */
static void check_var_arg_32(vpiHandle arg, vpiHandle callh,
                             const char *name, const char *desc)
{
      assert(arg);

      switch (vpi_get(vpiType, arg)) {
	case vpiMemoryWord:
	case vpiPartSelect:
	case vpiBitVar:
	case vpiReg: // Check that we have exactly 32 bits.
	    if (vpi_get(vpiSize, arg) != 32) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's %s (variable) argument must be 32 bits.\n",
		             name, desc);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }
	case vpiIntegerVar:
	case vpiIntVar:
	    break;
	default:
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's %s argument must be a 32 bit variable.\n",
	               name, desc);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }
}

/*
 * Check to see if the argument is a variable of at least 32 bits.
 */
static void check_var_arg_large(vpiHandle arg, vpiHandle callh,
                                const char *name, const char *desc)
{
      assert(arg);

      switch (vpi_get(vpiType, arg)) {
	case vpiMemoryWord:
	case vpiPartSelect:
	case vpiBitVar:
	case vpiReg: // Check that we have at least 32 bits.
	    if (vpi_get(vpiSize, arg) < 32) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's %s (variable) argument must have at least "
		             "32 bits.\n", name, desc);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }
	case vpiIntegerVar:
	case vpiIntVar:
	case vpiLongIntVar:
	case vpiTimeVar:
	    break;
	default:
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's %s argument must be a variable.\n", name, desc);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }
}

/*
 * Check that the given number of arguments are numeric.
 */
static unsigned check_numeric_args(vpiHandle argv, unsigned count,
                                   vpiHandle callh, const char *name)
{
      unsigned idx;

	/* Check that the first count arguments are numeric. Currently
	 * only three are needed/supported. */
      for (idx = 0; idx < count; idx += 1) {
	    const char *loc = NULL;
	    vpiHandle arg = vpi_scan(argv);

	      /* Get the name for this argument. */
	    switch (idx) {
	      case 0: loc = "first"; break;
	      case 1: loc = "second"; break;
	      case 2: loc = "third"; break;
	      default: assert(0);
	    }

	      /* Check that there actually is an argument. */
	    if (! arg) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s requires a %s (<= 32 bit numeric) argument.\n",
		             name, loc);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
		  return 1;
	    }

	      /* Check that it is no more than 32 bits. */
	    if (! is_32_or_smaller_obj(arg)) {
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's %s argument must be numeric (<= 32 bits).\n",
		             name, loc);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
	    }
      }

      return 0;
}

/*
 * Check to see if the given argument is valid (does not have any X/Z bits).
 * Return zero if it is valid and a positive value if it is invalid.
 */
static unsigned get_valid_32(vpiHandle arg, PLI_INT32 *value)
{
      PLI_INT32 size, mask;
      s_vpi_value val;

      size = vpi_get(vpiSize, arg);
	/* The compiletf routine should have already verified that this is
	 * <= 32 bits. */
      assert((size <= 32) && (size > 0));

	/* Create a mask so that we only check the appropriate bits. */
      mask = UINT32_MAX >> (32 - size);

	/* Get the value and return the possible integer value in the value
	 * variable. Return the b-value bits to indicate if the value is
	 * undefined (has X/Z bit). */
      val.format = vpiVectorVal;
      vpi_get_value(arg, &val);

      *value = val.value.vector->aval & mask;
	/* If the argument is signed and less than 32 bit we need to sign
	 * extend the value. */
      if (vpi_get(vpiSigned, arg) && (size < 32)) {
	    if ((*value) & (1 << (size - 1))) *value |= ~mask;
      }
      return (val.value.vector->bval & mask);
}

static void get_four_state(vpiHandle arg, p_vpi_vecval vec)
{
      PLI_INT32 size, mask;
      s_vpi_value val;

      size = vpi_get(vpiSize, arg);
	/* The compiletf routine should have already verified that this is
	 * <= 32 bits. */
      assert((size <= 32) && (size > 0));

	/* Create a mask so that we only use the appropriate bits. */
      mask = UINT32_MAX >> (32 - size);

	/* Get the bits for the argument and save them in the return value. */
      val.format = vpiVectorVal;
      vpi_get_value(arg, &val);
      vec->aval = val.value.vector->aval & mask;
      vec->bval = val.value.vector->bval & mask;

	/* If the argument is signed and less than 32 bit we need to sign
	 * extend the value. */
      if (vpi_get(vpiSigned, arg) && (size < 32)) {
	    if (vec->aval & (1 << (size - 1))) vec->aval |= ~mask;
	    if (vec->bval & (1 << (size - 1))) vec->bval |= ~mask;
      }
}

/*
 * Fill the passed variable with x.
 */
static void fill_variable_with_x(vpiHandle var)
{
      s_vpi_value val;
      PLI_INT32 words = ((vpi_get(vpiSize, var) - 1) / 32) + 1;
      PLI_INT32 idx;
      p_vpi_vecval val_ptr = (p_vpi_vecval) malloc(words*sizeof(s_vpi_vecval));

      assert(val_ptr);

	/* Fill the vector with X. */
      for (idx = 0; idx < words; idx += 1) {
	    val_ptr[idx].aval = 0xffffffff;
	    val_ptr[idx].bval = 0xffffffff;
      }

	/* Put the vector to the variable. */
      val.format = vpiVectorVal;
      val.value.vector = val_ptr;
      vpi_put_value(var, &val, 0, vpiNoDelay);
      free(val_ptr);
}

/*
 * Fill the passed variable with the passed value if it fits. If it doesn't
 * fit then set all bits to one and return that the value is too big instead
 * of the normal OK. The value is a time and needs to be scaled to the
 * calling module's timescale.
 */
static PLI_INT32 fill_variable_with_scaled_time(vpiHandle var, uint64_t c_time)
{
      s_vpi_value val;
      PLI_INT32 size = vpi_get(vpiSize, var);
      PLI_INT32 is_signed = vpi_get(vpiSigned, var);
      PLI_INT32 words = ((size - 1) / 32) + 1;
      uint64_t max_val = 0;
      uint64_t scale = 1;
      uint64_t frac;
      PLI_INT32 rtn, units, prec;
      p_vpi_vecval val_ptr = (p_vpi_vecval) malloc(words*sizeof(s_vpi_vecval));

      assert(val_ptr);
      assert(size >= 32);
      assert(words > 0);

	/* Scale the variable to match the calling module's timescale. */
      prec = vpi_get(vpiTimePrecision, 0);
      units = vpi_get(vpiTimeUnit, vpi_handle(vpiModule, var));
      assert(units >= prec);
      while (units > prec) {
	    scale *= 10;
	    units -= 1;
      }
      frac = c_time % scale;
      c_time /= scale;
      if ((scale > 1) && (frac >= scale/2)) c_time += 1;

	/* Find the maximum value + 1 that can be put into the variable. */
      if (size < 64) {
	    max_val = 1;
	    max_val <<= (size - is_signed);
      }

	/* If the time is too big to fit then return the maximum positive
	 * value and that the value overflowed. Otherwise, return the time
	 * and OK. */
      if (max_val && (c_time >= max_val)) {
	      /* For a single word only the MSB is cleared if signed. */
	    if (words == 1) {
		  if (is_signed) {
			val_ptr[0].aval = 0x7fffffff;
		  } else {
			val_ptr[0].aval = 0xffffffff;
		  }
		  val_ptr[0].bval = 0x00000000;
	      /* For two words the lower word is filled with 1 and the top
	       * word has a size dependent fill if signed. */
	    } else {
		  assert(words == 2);
		  val_ptr[0].aval = 0xffffffff;
		  val_ptr[0].bval = 0x00000000;
		  if (is_signed) {
			val_ptr[1].aval = ~(UINT32_MAX >> (size - 32));
		  } else {
			val_ptr[1].aval = 0xffffffff;
		  }
		  val_ptr[1].bval = 0x00000000;
	    }
	    rtn = IVL_QUEUE_VALUE_OVERFLOWED;
      } else {
	      /* Fill the vector with 0. */
	    for (PLI_INT32 idx = 0; idx < words; idx += 1) {
		  val_ptr[idx].aval = 0x00000000;
		  val_ptr[idx].bval = 0x00000000;
	    }
	      /* Add the time to the vector. */
	    switch (words) {
	      default:
		  val_ptr[1].aval = (c_time >> 32) & 0xffffffff;
		  // fallthrough
	      case 1:
		  val_ptr[0].aval = c_time & 0xffffffff;
	    }
	    rtn = IVL_QUEUE_OK;
      }

	/* Put the vector to the variable. */
      val.format = vpiVectorVal;
      val.value.vector = val_ptr;
      vpi_put_value(var, &val, 0, vpiNoDelay);
      free(val_ptr);

      return rtn;
}

/*
 * Check that the given $q_initialize() call has valid arguments.
 */
static PLI_INT32 sys_q_initialize_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires four arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Check that the first three arguments (the id, type and maximum
	 * length) are numeric. */
      if (check_numeric_args(argv, 3, callh, name)) return 0;

	/* The fourth argument (the status) must be a variable. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a fourth (variable) argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
	/* Check that the status argument is a 32 bit variable. */
      check_var_arg_32(arg, callh, name, "fourth");

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "four arguments", 0);

      return 0;
}

/*
 * The runtime code for $q_initialize().
 */
static PLI_INT32 sys_q_initialize_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle status;
      PLI_INT32 id, type, length;
      s_vpi_value val;
      unsigned invalid_id, invalid_type, invalid_length;

      (void)name; /* Parameter is not used. */

	/* Get the id. */
      invalid_id = get_valid_32(vpi_scan(argv), &id);

	/* Get the queue type. */
      invalid_type = get_valid_32(vpi_scan(argv), &type);

	/* Get the queue maximum length. */
      invalid_length = get_valid_32(vpi_scan(argv), &length);

	/* Get the status variable. */
      status = vpi_scan(argv);

	/* We are done with the argument iterator so free it. */
      vpi_free_object(argv);

	/* If the id is invalid then return. */
      if (invalid_id) {
	    val.format = vpiIntVal;
	    val.value.integer = IVL_QUEUE_UNDEFINED_ID;
	    vpi_put_value(status, &val, 0, vpiNoDelay);
	    return 0;
      }

	/* Verify that the type is valid. */
      if (invalid_type || ((type != IVL_QUEUE_FIFO) &&
                           (type != IVL_QUEUE_LIFO))) {
	    val.format = vpiIntVal;
	    val.value.integer = IVL_QUEUE_UNSUPPORTED_TYPE;
	    vpi_put_value(status, &val, 0, vpiNoDelay);
	    return 0;
      }

	/* Verify that the queue length is greater than zero. */
      if (invalid_length || (length <= 0)) {
	    val.format = vpiIntVal;
	    val.value.integer = IVL_QUEUE_INVALID_LENGTH;
	    vpi_put_value(status, &val, 0, vpiNoDelay);
	    return 0;
      }

	/* Check that this is not a duplicate queue id. */
      if (get_id_index(id) >= 0) {
	    val.format = vpiIntVal;
	    val.value.integer = IVL_QUEUE_DUPLICATE_ID;
	    vpi_put_value(status, &val, 0, vpiNoDelay);
	    return 0;
      }

	/* Create the queue and fail if we do not have enough memory. */
      if (create_queue(id, type, length)) {
	    val.format = vpiIntVal;
	    val.value.integer = IVL_QUEUE_OUT_OF_MEMORY;
	    vpi_put_value(status, &val, 0, vpiNoDelay);
	    return 0;
      }

	/* The queue was initialized correctly so return OK. */
      val.format = vpiIntVal;
      val.value.integer = IVL_QUEUE_OK;
      vpi_put_value(status, &val, 0, vpiNoDelay);
      return 0;
}

/*
 * Check that the given $q_add() call has valid arguments.
 */
static PLI_INT32 sys_q_add_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires four arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Check that the first three arguments (the id, job and information)
	 * are numeric. */
      if (check_numeric_args(argv, 3, callh, name)) return 0;

	/* The fourth argument (the status) must be a variable. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a fourth (variable) argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
	/* Check that the status argument is a 32 bit variable. */
      check_var_arg_32(arg, callh, name, "fourth");

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "four arguments", 0);

      return 0;
}

/*
 * The runtime code for $q_add().
 */
static PLI_INT32 sys_q_add_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle status;
      PLI_INT32 id;
      int64_t idx;
      s_vpi_vecval job, inform;
      s_vpi_value val;
      unsigned invalid_id;

      (void)name; /* Parameter is not used. */

	/* Get the id. */
      invalid_id = get_valid_32(vpi_scan(argv), &id);

	/* Get the job. */
      get_four_state(vpi_scan(argv), &job);

	/* Get the value. */
      get_four_state(vpi_scan(argv), &inform);

	/* Get the status variable. */
      status = vpi_scan(argv);

	/* We are done with the argument iterator so free it. */
      vpi_free_object(argv);

	/* Verify that the id is valid. */
      idx = get_id_index(id);
      if (invalid_id || (idx < 0)) {
	    val.format = vpiIntVal;
	    val.value.integer = IVL_QUEUE_UNDEFINED_ID;
	    vpi_put_value(status, &val, 0, vpiNoDelay);
	    return 0;
      }

	/* Add the data to the queue if it is not already full. */
      if (add_to_queue(idx, &job, &inform)) {
	    val.format = vpiIntVal;
	    val.value.integer = IVL_QUEUE_FULL;
	    vpi_put_value(status, &val, 0, vpiNoDelay);
	    return 0;
      }

	/* The data was added to the queue so return OK. */
      val.format = vpiIntVal;
      val.value.integer = IVL_QUEUE_OK;
      vpi_put_value(status, &val, 0, vpiNoDelay);
      return 0;
}

/*
 * Check that the given $q_remove() call has valid arguments.
 */
static PLI_INT32 sys_q_remove_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires four arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* The first argument (the id) must be numeric. */
      if (! is_32_or_smaller_obj(vpi_scan(argv))) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's first argument must be numeric (<= 32 bits).\n",
	               name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* The second argument (the job id) must be a variable. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a second (variable) argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
	/* Check that the job id argument is a 32 bit variable. */
      check_var_arg_32(arg, callh, name, "second");

	/* The third argument (the information id) must be a variable. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a third (variable) argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
	/* Check that the information id argument is a 32 bit variable. */
      check_var_arg_32(arg, callh, name, "third");

	/* The fourth argument (the status) must be a variable. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a fourth (variable) argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
	/* Check that the status argument is a 32 bit variable. */
      check_var_arg_32(arg, callh, name, "fourth");

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "four arguments", 0);

      return 0;
}

/*
 * The runtime code for $q_remove().
 */
static PLI_INT32 sys_q_remove_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle job, inform, status;
      PLI_INT32 id, idx;
      s_vpi_vecval job_val, inform_val;
      s_vpi_value val;
      unsigned invalid_id;

      (void)name; /* Parameter is not used. */

	/* Get the id. */
      invalid_id = get_valid_32(vpi_scan(argv), &id);

	/* Get the job variable. */
      job = vpi_scan(argv);

	/* Get the inform variable. */
      inform = vpi_scan(argv);

	/* Get the status variable. */
      status = vpi_scan(argv);

	/* We are done with the argument iterator so free it. */
      vpi_free_object(argv);

	/* Verify that the id is valid. */
      idx = get_id_index(id);
      if (invalid_id || (idx < 0)) {
	    fill_variable_with_x(job);
	    fill_variable_with_x(inform);
	    val.format = vpiIntVal;
	    val.value.integer = IVL_QUEUE_UNDEFINED_ID;
	    vpi_put_value(status, &val, 0, vpiNoDelay);
	    return 0;
      }

	/* Remove the data from the queue if it is not already empty. */
      if (remove_from_queue(idx, &job_val, &inform_val)) {
	    fill_variable_with_x(job);
	    fill_variable_with_x(inform);
	    val.format = vpiIntVal;
	    val.value.integer = IVL_QUEUE_EMPTY;
	    vpi_put_value(status, &val, 0, vpiNoDelay);
	    return 0;
      }

      val.format = vpiVectorVal;
      val.value.vector = &job_val;
      vpi_put_value(job, &val, 0, vpiNoDelay);
      val.format = vpiVectorVal;
      val.value.vector = &inform_val;
      vpi_put_value(inform, &val, 0, vpiNoDelay);

	/* The data was added to the queue so return OK. */
      val.format = vpiIntVal;
      val.value.integer = IVL_QUEUE_OK;
      vpi_put_value(status, &val, 0, vpiNoDelay);
      return 0;
}

/*
 * Check that the given $q_full() call has valid arguments.
 */
static PLI_INT32 sys_q_full_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires two arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* The first argument (the id) must be numeric. */
      if (! is_32_or_smaller_obj(vpi_scan(argv))) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's first argument must be numeric (<= 32 bits).\n",
	               name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* The second argument (the status) must be a variable. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a second (variable) argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
	/* Check that the status argument is a 32 bit variable. */
      check_var_arg_32(arg, callh, name, "second");

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "two arguments", 0);

      return 0;
}

/*
 * The runtime code for $q_full().
 */
static PLI_INT32 sys_q_full_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle status;
      PLI_INT32 id, idx;
      s_vpi_value val;
      unsigned invalid_id;

      (void)name; /* Parameter is not used. */

	/* Get the id. */
      invalid_id = get_valid_32(vpi_scan(argv), &id);

	/* Get the status variable. */
      status = vpi_scan(argv);

	/* We are done with the argument iterator so free it. */
      vpi_free_object(argv);

	/* Verify that the id is valid. */
      idx = get_id_index(id);
      if (invalid_id || (idx < 0)) {
	    val.format = vpiIntVal;
	    val.value.integer = IVL_QUEUE_UNDEFINED_ID;
	    vpi_put_value(status, &val, 0, vpiNoDelay);
	    fill_variable_with_x(callh);
	    return 0;
      }

	/* Get the queue state and return it. */
      val.format = vpiIntVal;
      if (is_queue_full(idx)) val.value.integer = 1;
      else val.value.integer = 0;
      vpi_put_value(callh, &val, 0, vpiNoDelay);

	/* The queue state was passed back so return OK. */
      val.format = vpiIntVal;
      val.value.integer = IVL_QUEUE_OK;
      vpi_put_value(status, &val, 0, vpiNoDelay);
      return 0;
}

/*
 * Check that the given $q_exam() call has valid arguments.
 */
static PLI_INT32 sys_q_exam_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

	/* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires four arguments.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

	/* Check that the first two arguments (the id and code) are numeric. */
      if (check_numeric_args(argv, 2, callh, name)) return 0;

	/* The third argument (the value) must be a variable. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a third (variable) argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
	/* Check that the value argument is a variable with at least
	 * 32 bits. */
      check_var_arg_large(arg, callh, name, "third");

	/* The fourth argument (the status) must be a variable. */
      arg = vpi_scan(argv);
      if (! arg) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a fourth (variable) argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
	/* Check that the status argument is a 32 bit variable. */
      check_var_arg_32(arg, callh, name, "fourth");

      /* Make sure there are no extra arguments. */
      check_for_extra_args(argv, callh, name, "two arguments", 0);

      return 0;
}

/*
 * The runtime code for $q_exam().
 */
static PLI_INT32 sys_q_exam_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle value, status;
      PLI_INT32 id, code, idx, rtn;
      s_vpi_value val;
      unsigned invalid_id, invalid_code;

      (void)name; /* Parameter is not used. */

	/* Get the id. */
      invalid_id = get_valid_32(vpi_scan(argv), &id);

	/* Get the code. */
      invalid_code = get_valid_32(vpi_scan(argv), &code);

	/* Get the value variable. */
      value = vpi_scan(argv);

	/* Get the status variable. */
      status = vpi_scan(argv);

	/* We are done with the argument iterator so free it. */
      vpi_free_object(argv);

	/* Verify that the id is valid. */
      idx = get_id_index(id);
      if (invalid_id || (idx < 0)) {
	    fill_variable_with_x(value);
	    val.format = vpiIntVal;
	    val.value.integer = IVL_QUEUE_UNDEFINED_ID;
	    vpi_put_value(status, &val, 0, vpiNoDelay);
	    return 0;
      }

	/* Verify that the code is valid. */
      if (invalid_code || (code <= 0) || (code > 6)) {
	    fill_variable_with_x(value);
	    val.format = vpiIntVal;
	    val.value.integer = IVL_QUEUE_UNDEFINED_STAT_CODE;
	    vpi_put_value(status, &val, 0, vpiNoDelay);
	    return 0;
      }

      rtn = IVL_QUEUE_OK;

	/* Calculate the requested queue information. */
      switch (code) {
	  /* The current queue length. */
	case IVL_QUEUE_LENGTH:
	    val.format = vpiIntVal;
	    val.value.integer = get_current_queue_length(idx);
	    vpi_put_value(value, &val, 0, vpiNoDelay);
	    break;
	  /* The mean inter-arrival time. */
	case IVL_QUEUE_MEAN:
	    if (have_interarrival_statistic(idx) == 0) {
		  fill_variable_with_x(value);
		  rtn = IVL_QUEUE_NO_STATISTICS;
	    } else {
		  uint64_t ia_time = get_mean_interarrival_time(idx);
		  rtn = fill_variable_with_scaled_time(value, ia_time);
	    }
	    break;
	  /* The maximum queue length. */
	case IVL_QUEUE_MAX_LENGTH:
	    val.format = vpiIntVal;
	    val.value.integer = get_maximum_queue_length(idx);
	    vpi_put_value(value, &val, 0, vpiNoDelay);
	    break;
	  /* The shortest queue wait time ever. */
	case IVL_QUEUE_SHORTEST:
	    if (have_shortest_wait_statistic(idx) == 0) {
		  fill_variable_with_x(value);
		  rtn = IVL_QUEUE_NO_STATISTICS;
	    } else {
		  uint64_t sw_time = get_shortest_wait_time(idx);
		  rtn = fill_variable_with_scaled_time(value, sw_time);
	    }
	    break;
	  /* The longest wait time for elements still in the queue. */
	case IVL_QUEUE_LONGEST:
	    if (get_current_queue_length(idx) == 0) {
		  fill_variable_with_x(value);
		  rtn = IVL_QUEUE_NO_STATISTICS;
	    } else {
		  uint64_t lq_time = get_longest_queue_time(idx);
		  rtn = fill_variable_with_scaled_time(value, lq_time);
	    }
	    break;
	  /* The average queue wait time. */
	case IVL_QUEUE_AVERAGE:
	    if (have_average_wait_statistic(idx) == 0) {
		  fill_variable_with_x(value);
		  rtn = IVL_QUEUE_NO_STATISTICS;
	    } else {
		  uint64_t aw_time = get_average_wait_time(idx);
		  rtn = fill_variable_with_scaled_time(value, aw_time);
	    }
	    break;
	default:
	    assert(0);
      }

	/* The queue information was passed back so now return the status. */
      val.format = vpiIntVal;
      val.value.integer = rtn;
      vpi_put_value(status, &val, 0, vpiNoDelay);
      return 0;
}

/*
 * Routine to register the system tasks/functions provided in this file.
 */
void sys_queue_register(void)
{
      s_vpi_systf_data tf_data;
      s_cb_data cb;
      vpiHandle res;

      tf_data.type = vpiSysTask;
      tf_data.tfname = "$q_initialize";
      tf_data.calltf = sys_q_initialize_calltf;
      tf_data.compiletf = sys_q_initialize_compiletf;
      tf_data.sizetf = 0;
      tf_data.user_data = "$q_initialize";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type = vpiSysTask;
      tf_data.tfname = "$q_add";
      tf_data.calltf = sys_q_add_calltf;
      tf_data.compiletf = sys_q_add_compiletf;
      tf_data.sizetf = 0;
      tf_data.user_data = "$q_add";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type = vpiSysTask;
      tf_data.tfname = "$q_remove";
      tf_data.calltf = sys_q_remove_calltf;
      tf_data.compiletf = sys_q_remove_compiletf;
      tf_data.sizetf = 0;
      tf_data.user_data = "$q_remove";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$q_full";
      tf_data.calltf = sys_q_full_calltf;
      tf_data.compiletf = sys_q_full_compiletf;
      tf_data.sizetf = 0;  /* Not needed for a vpiSysFuncInt. */
      tf_data.user_data = "$q_full";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type = vpiSysTask;
      tf_data.tfname = "$q_exam";
      tf_data.calltf = sys_q_exam_calltf;
      tf_data.compiletf = sys_q_exam_compiletf;
      tf_data.sizetf = 0;
      tf_data.user_data = "$q_exam";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

	/* Create a callback to clear all the queue memory when the
	 * simulator finishes. */
      cb.time = NULL;
      cb.reason = cbEndOfSimulation;
      cb.cb_rtn = cleanup_queue;
      cb.user_data = 0x0;
      cb.obj = 0x0;

      vpi_register_cb(&cb);
}
