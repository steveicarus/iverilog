#ifndef IVL_event_H
#define IVL_event_H
/*
 * Copyright (c) 2004-2021 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "vvp_net.h"
# include  "array.h"
# include  "vthread.h"
# include  "config.h"

class evctl {

    public:
      explicit evctl(unsigned long ecount);
      bool dec_and_run();
      virtual void run_run() = 0;
      virtual ~evctl() {}
      evctl*next;

    private:
      unsigned long ecount_;
};

class evctl_real : public evctl {

    public:
      explicit evctl_real(class __vpiHandle*handle, double value,
                          unsigned long ecount);
      virtual ~evctl_real() {}
      void run_run();

    private:
      class __vpiHandle*handle_;
      double value_;
};

class evctl_vector : public evctl {

    public:
      explicit evctl_vector(vvp_net_ptr_t ptr, const vvp_vector4_t&value,
                            unsigned off, unsigned wid, unsigned long ecount);
      virtual ~evctl_vector() {}
      void run_run();

    private:
      vvp_net_ptr_t ptr_;
      vvp_vector4_t value_;
      unsigned off_;
      unsigned wid_;
};

class evctl_array : public evctl {

    public:
      explicit evctl_array(vvp_array_t memory, unsigned index,
                           const vvp_vector4_t&value, unsigned off,
                           unsigned long ecount);
      virtual ~evctl_array() {}
      virtual void run_run();

    private:
      vvp_array_t mem_;
      unsigned idx_;
      vvp_vector4_t value_;
      unsigned off_;
};

class evctl_array_r : public evctl {

    public:
      explicit evctl_array_r(vvp_array_t memory, unsigned index,
                           double value, unsigned long ecount);
      virtual ~evctl_array_r() {}
      virtual void run_run();

    private:
      vvp_array_t mem_;
      unsigned idx_;
      double value_;
};

extern void schedule_evctl(class __vpiHandle*handle, double value,
                           vvp_net_t*event, unsigned long ecount);

extern void schedule_evctl(vvp_net_ptr_t ptr, const vvp_vector4_t&value,
                           unsigned offset, unsigned wid,
                           vvp_net_t*event, unsigned long ecount);

extern void schedule_evctl(vvp_array_t memory, unsigned index,
                           const vvp_vector4_t&value, unsigned offset,
                           vvp_net_t*event, unsigned long ecount);

extern void schedule_evctl(vvp_array_t memory, unsigned index,
                           double value,
                           vvp_net_t*event, unsigned long ecount);

/*
 *  Event / edge detection functors
 */

/*
 * A "waitable" functor is one that the %wait instruction can wait
 * on. This includes the infrastructure needed to hold threads.
 */
struct waitable_hooks_s {

    public:
      waitable_hooks_s() : event_ctls(0) { last = &event_ctls; }
      virtual ~waitable_hooks_s() {}

      virtual vthread_t add_waiting_thread(vthread_t thread) = 0;

      evctl*event_ctls;
      evctl**last;

    protected:
      void run_waiting_threads_(vthread_t&threads);
};

/*
 * This is the base object for storing state information for each instance
 * of an automatically allocated event. In the general case, all that is
 * needed is the list of threads waiting on that instance.
 */
struct waitable_state_s {
      waitable_state_s() : threads(0) {}

      vthread_t threads;
};

/*
 * The vvp_fun_edge functor detects events that are edges of various
 * types. This should be hooked to a vvp_net_t that is connected to
 * the output of a signal that we wish to watch for edges.
 */
class vvp_fun_edge : public vvp_net_fun_t, public waitable_hooks_s {

    public:
      typedef unsigned short edge_t;
      explicit vvp_fun_edge(edge_t e);
      virtual ~vvp_fun_edge();

    protected:
      bool recv_vec4_(const vvp_vector4_t&bit,
                      vvp_bit4_t&old_bit, vthread_t&threads);

      vvp_bit4_t bits_[4];

    private:
      edge_t edge_;
};

extern const vvp_fun_edge::edge_t vvp_edge_edge;
extern const vvp_fun_edge::edge_t vvp_edge_posedge;
extern const vvp_fun_edge::edge_t vvp_edge_negedge;
extern const vvp_fun_edge::edge_t vvp_edge_none;

/*
 * Statically allocated vvp_fun_edge.
 */
class vvp_fun_edge_sa : public vvp_fun_edge {

    public:
      explicit vvp_fun_edge_sa(edge_t e);
      virtual ~vvp_fun_edge_sa();

      vthread_t add_waiting_thread(vthread_t thread);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);
      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t context);

    private:
      vthread_t threads_;
};

/*
 * Automatically allocated vvp_fun_edge.
 */
class vvp_fun_edge_aa : public vvp_fun_edge, public automatic_hooks_s {

    public:
      explicit vvp_fun_edge_aa(edge_t e);
      virtual ~vvp_fun_edge_aa();

      void alloc_instance(vvp_context_t context);
      void reset_instance(vvp_context_t context);
#ifdef CHECK_WITH_VALGRIND
      void free_instance(vvp_context_t context);
#endif

      vthread_t add_waiting_thread(vthread_t thread);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

    private:
      __vpiScope*context_scope_;
      unsigned context_idx_;
};

/*
 * This is the base object for storing the last received values for a
 * vvp_fun_anyedge functor.
 */
class anyedge_value;

/*
 * The vvp_fun_anyedge functor checks to see if any value in an input
 * vector changes. Unlike the vvp_fun_edge, which watches for the LSB
 * of its inputs to change in a particular direction, the anyedge
 * functor looks at the entire input vector for any change.
 *
 * The anyedge is also different in that it can receive real
 * values. In this case, any detectable change in the real value leads
 * to an event trigger.
 */
class vvp_fun_anyedge : public vvp_net_fun_t, public waitable_hooks_s {

    public:
      explicit vvp_fun_anyedge();
      virtual ~vvp_fun_anyedge();

    protected:
      anyedge_value*last_value_[4];
};

/*
 * Statically allocated vvp_fun_anyedge.
 */
class vvp_fun_anyedge_sa : public vvp_fun_anyedge {

    public:
      explicit vvp_fun_anyedge_sa();
      virtual ~vvp_fun_anyedge_sa();

      vthread_t add_waiting_thread(vthread_t thread);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);
      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t context);

      void recv_real(vvp_net_ptr_t port, double bit,
                     vvp_context_t context);

      void recv_string(vvp_net_ptr_t port, const std::string&bit,
		       vvp_context_t context);
      void recv_object(vvp_net_ptr_t port, vvp_object_t bit,
		       vvp_context_t context);

    private:
      vthread_t threads_;
};

/*
 * Automatically allocated vvp_fun_anyedge.
 */
class vvp_fun_anyedge_aa : public vvp_fun_anyedge, public automatic_hooks_s {

    public:
      explicit vvp_fun_anyedge_aa();
      virtual ~vvp_fun_anyedge_aa();

      void alloc_instance(vvp_context_t context);
      void reset_instance(vvp_context_t context);
#ifdef CHECK_WITH_VALGRIND
      void free_instance(vvp_context_t context);
#endif

      vthread_t add_waiting_thread(vthread_t thread);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

      void recv_real(vvp_net_ptr_t port, double bit,
                     vvp_context_t context);

      void recv_string(vvp_net_ptr_t port, const std::string&bit,
		       vvp_context_t context);

    private:
      __vpiScope*context_scope_;
      unsigned context_idx_;
};

/*
 * This functor triggers anytime any input is set, no matter what the
 * value. This is similar to a named event, but it has no handle. It
 * supports wide (more than 4) inputs, so needs a pointer to the base
 * net that is used for output.
 */
class vvp_fun_event_or : public vvp_net_fun_t, public waitable_hooks_s {

    public:
      explicit vvp_fun_event_or(vvp_net_t*base_net);
      ~vvp_fun_event_or();

    protected:
      vvp_net_t*base_net_;
};

/*
 * Statically allocated vvp_fun_event_or.
 */
class vvp_fun_event_or_sa : public vvp_fun_event_or {

    public:
      explicit vvp_fun_event_or_sa(vvp_net_t*base_net);
      ~vvp_fun_event_or_sa();

      vthread_t add_waiting_thread(vthread_t thread);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

    private:
      vthread_t threads_;
};

/*
 * Automatically allocated vvp_fun_event_or.
 */
class vvp_fun_event_or_aa : public vvp_fun_event_or, public automatic_hooks_s {

    public:
      explicit vvp_fun_event_or_aa(vvp_net_t*base_net);
      ~vvp_fun_event_or_aa();

      void alloc_instance(vvp_context_t context);
      void reset_instance(vvp_context_t context);
#ifdef CHECK_WITH_VALGRIND
      void free_instance(vvp_context_t context);
#endif

      vthread_t add_waiting_thread(vthread_t thread);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

    private:
      __vpiScope*context_scope_;
      unsigned context_idx_;
};

/*
 * A named event is simpler than a vvp_fun_edge in that it triggers on
 * any input at all to port-0. The idea here is that behavioral code
 * can use a %set/v instruction to trigger the event.
 */
class vvp_named_event : public vvp_net_fun_t, public waitable_hooks_s {

    public:
      explicit vvp_named_event(class __vpiHandle*eh);
      ~vvp_named_event();

    protected:
      class __vpiHandle*handle_;
};

/*
 * Statically allocated vvp_named_event.
 */
class vvp_named_event_sa : public vvp_named_event {

    public:
      explicit vvp_named_event_sa(class __vpiHandle*eh);
      ~vvp_named_event_sa();

      vthread_t add_waiting_thread(vthread_t thread);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t);

    private:
      vthread_t threads_;
};

/*
 * Automatically allocated vvp_named_event.
 */
class vvp_named_event_aa : public vvp_named_event, public automatic_hooks_s {

    public:
      explicit vvp_named_event_aa(class __vpiHandle*eh);
      ~vvp_named_event_aa();

      void alloc_instance(vvp_context_t context);
      void reset_instance(vvp_context_t context);
#ifdef CHECK_WITH_VALGRIND
      void free_instance(vvp_context_t context);
#endif

      vthread_t add_waiting_thread(vthread_t thread);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

    private:
      unsigned context_idx_;
};

#endif /* IVL_event_H */
