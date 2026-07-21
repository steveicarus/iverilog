/*
 * Copyright (c) 2026 Icarus UVM fork contributors
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 * Runtime implementation of SystemVerilog built-in mailbox and semaphore.
 */

#include "vvp_mailbox.h"
#include "vthread.h"
#include "schedule.h"
#include <cassert>

/* ================================================================
 * vvp_mailbox
 * ================================================================ */

vvp_mailbox::vvp_mailbox(size_t bound)
: bound_(bound)
{
}

vvp_mailbox::~vvp_mailbox()
{
}

vvp_object* vvp_mailbox::duplicate() const
{
      vvp_mailbox* copy = new vvp_mailbox(bound_);
      copy->items_ = items_;
      /* Note: waiters are NOT copied – a duplicate mailbox has no
       * pending waiters. */
      return copy;
}

bool vvp_mailbox::try_put(const vvp_object_t& item)
{
      if (full())
	    return false;
      items_.push_back(item);
      resume_get_waiters_();
      return true;
}

bool vvp_mailbox::try_get(vvp_object_t& item)
{
      if (empty())
	    return false;
      item = items_.front();
      items_.pop_front();
      resume_put_waiters_();
      return true;
}

bool vvp_mailbox::try_peek(vvp_object_t& item)
{
      if (empty())
	    return false;
      item = items_.front();
      return true;
}

bool vvp_mailbox::put(vthread_t thr, const vvp_object_t& item)
{
      if (!full()) {
	    items_.push_back(item);
	    resume_get_waiters_();
	    return true;  /* completed immediately */
      }
      /* Mailbox is full – suspend the thread. Store the item in the
       * waiter record so it is available when space opens up. */
      put_waiter_t w;
      w.thr  = thr;
      w.item = item;
      put_waiters_.push_back(w);
      return false;             /* thread suspended */
}

bool vvp_mailbox::get(vthread_t thr, vvp_object_t& item_out)
{
      if (!empty()) {
	    item_out = items_.front();
	    items_.pop_front();
	    resume_put_waiters_();
	    return true;
      }
      get_waiter_t w;
      w.thr = thr;
      w.is_peek = false;
      get_waiters_.push_back(w);
      return false;
}

bool vvp_mailbox::peek(vthread_t thr, vvp_object_t& item_out)
{
      if (!empty()) {
	    item_out = items_.front();
	    return true;
      }
      get_waiter_t w;
      w.thr = thr;
      w.is_peek = true;
      get_waiters_.push_back(w);
      return false;
}

void vvp_mailbox::resume_get_waiters_()
{
      while (!get_waiters_.empty() && !empty()) {
	    get_waiter_t waiter = get_waiters_.front();
	    get_waiters_.erase(get_waiters_.begin());
	    /* Push the retrieved item onto the waiting thread's object
	     * stack so it is available when the thread resumes at the
	     * instruction following %mbx/get or %mbx/peek. */
	    vthread_push_obj_item(waiter.thr, items_.front());
	    if (!waiter.is_peek) {
		  items_.pop_front();
		  resume_put_waiters_();
	    }
	    schedule_vthread(waiter.thr, 0, true);
      }
}

void vvp_mailbox::resume_put_waiters_()
{
      while (!put_waiters_.empty() && !full()) {
	    put_waiter_t w = put_waiters_.front();
	    put_waiters_.erase(put_waiters_.begin());
	    items_.push_back(w.item);
	    schedule_vthread(w.thr, 0, true);
	    resume_get_waiters_();
      }
}

/* ================================================================
 * vvp_semaphore
 * ================================================================ */

vvp_semaphore::vvp_semaphore(size_t initial_count)
: count_(initial_count)
{
}

vvp_semaphore::~vvp_semaphore()
{
}

vvp_object* vvp_semaphore::duplicate() const
{
      return new vvp_semaphore(count_);
}

bool vvp_semaphore::try_get(size_t n)
{
      if (count_ >= n) {
	    count_ -= n;
	    return true;
      }
      return false;
}

void vvp_semaphore::put(size_t n)
{
      count_ += n;
      resume_waiters_();
}

bool vvp_semaphore::get(vthread_t thr, size_t n)
{
      if (count_ >= n) {
	    count_ -= n;
	    return true;
      }
      waiter_t w;
      w.thr = thr;
      w.n   = n;
      waiters_.push_back(w);
      return false;
}

void vvp_semaphore::resume_waiters_()
{
      bool progress = true;
      while (progress) {
	    progress = false;
	    for (size_t idx = 0; idx < waiters_.size(); ++idx) {
		  if (count_ >= waiters_[idx].n) {
			count_ -= waiters_[idx].n;
			vthread_t w = waiters_[idx].thr;
			waiters_.erase(waiters_.begin() + (long)idx);
			schedule_vthread(w, 0, true);
			progress = true;
			break;
		  }
	    }
      }
}
