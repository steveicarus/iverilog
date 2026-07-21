/*
 * Copyright (c) 2026 Icarus UVM fork contributors
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 * Runtime objects for SystemVerilog built-in mailbox and semaphore.
 */
#ifndef VVP_MAILBOX_H
#define VVP_MAILBOX_H

#include "vvp_object.h"
#include "vvp_net.h"
#include <deque>
#include <vector>

struct vthread_s;
typedef vthread_s* vthread_t;

/*
 * vvp_mailbox -- runtime representation of SystemVerilog's
 * built-in parameterized mailbox.
 *
 * A mailbox is an unbounded (or bounded) typed FIFO. When the FIFO
 * is empty, get() and peek() suspend the calling thread.  When the
 * FIFO is at capacity, put() suspends the calling thread.
 *
 * We store contents as vvp_object_t (like queue_object). String and
 * vec4 mailboxes would need separate classes; for UVM the mailbox
 * element type is always an object.
 */
class vvp_mailbox : public vvp_object {
    public:
      explicit vvp_mailbox(size_t bound = 0);  /* 0 = unbounded */
      ~vvp_mailbox() override;

      vvp_object* duplicate() const override;
      void shallow_copy(const vvp_object*) override {}

      size_t bound() const { return bound_; }
      size_t num()   const { return items_.size(); }
      bool   full()  const { return bound_ && items_.size() >= bound_; }
      bool   empty() const { return items_.empty(); }

      /* Non-blocking operations.  Return true on success. */
      bool try_put(const vvp_object_t& item);
      bool try_get(vvp_object_t& item);
      bool try_peek(vvp_object_t& item);

      /*
       * Blocking operations: return true if the operation completed
       * immediately, or false if thr was suspended and will be
       * rescheduled when the condition changes.
       *
       * For get/peek: when returning false the thread is added to
       * get_waiters_.  On resumption (via resume_get_waiters_) the
       * retrieved item has already been pushed onto the thread's
       * object stack by vthread_push_obj_item().
       *
       * For put: when returning false the item is stored in the
       * put_waiter_t record; it is NOT pushed to the thread stack.
       */
      bool put(vthread_t thr, const vvp_object_t& item);
      bool get(vthread_t thr, vvp_object_t& item_out);
      bool peek(vthread_t thr, vvp_object_t& item_out);

    private:
      void resume_get_waiters_();
      void resume_put_waiters_();

      size_t bound_;                      /* 0 = unbounded */
      std::deque<vvp_object_t> items_;

      /* Threads suspended waiting for items (get/peek). */
      struct get_waiter_t {
	    vthread_t thr;
	    bool is_peek;
      };
      std::vector<get_waiter_t> get_waiters_;

      /* Threads suspended waiting for space (put).
       * The item to be put is stored here until space is available. */
      struct put_waiter_t {
	    vthread_t thr;
	    vvp_object_t item;
      };
      std::vector<put_waiter_t> put_waiters_;
};

/*
 * vvp_semaphore -- runtime representation of SystemVerilog's
 * built-in semaphore.
 *
 * A semaphore has an integer key count. get(N) blocks until at least
 * N keys are available, then decrements by N.  put(N) adds N keys
 * and wakes any waiters.  try_get(N) is non-blocking.
 */
class vvp_semaphore : public vvp_object {
    public:
      explicit vvp_semaphore(size_t initial_count = 0);
      ~vvp_semaphore() override;

      vvp_object* duplicate() const override;
      void shallow_copy(const vvp_object*) override {}

      size_t count() const { return count_; }

      /* try_get: if count >= n, decrement and return true; else false. */
      bool try_get(size_t n = 1);

      /* put: add n keys and wake suspended threads. */
      void put(size_t n = 1);

      /* get: block thr until count >= n, then decrement.
       * Returns true if immediately satisfied, false if thr is suspended. */
      bool get(vthread_t thr, size_t n = 1);

    private:
      void resume_waiters_();

      size_t count_;

      struct waiter_t {
	    vthread_t thr;
	    size_t n;
      };
      std::vector<waiter_t> waiters_;
};

/*
 * vvp_boxed_vec4 -- wraps a vec4 (integer/bit) value as a vvp_object_t so
 * that it can be stored in a mailbox alongside class objects.  Used when
 * mailbox #(T) is instantiated with a non-class element type (e.g. int).
 */
class vvp_boxed_vec4 : public vvp_object {
    public:
      explicit vvp_boxed_vec4(const vvp_vector4_t& v) : value_(v) {}
      ~vvp_boxed_vec4() override {}

      vvp_object* duplicate() const override {
	    return new vvp_boxed_vec4(value_);
      }
      void shallow_copy(const vvp_object* src) override {
	    if (const vvp_boxed_vec4* s = dynamic_cast<const vvp_boxed_vec4*>(src))
		  value_ = s->value_;
      }

      const vvp_vector4_t& get_value() const { return value_; }

    private:
      vvp_vector4_t value_;
};

#endif /* VVP_MAILBOX_H */
