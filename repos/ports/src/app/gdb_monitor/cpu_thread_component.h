/*
 * \brief  Cpu_thread_component class for GDB monitor
 * \author Christian Prochaska
 * \date   2016-05-12
 */

/*
 * Copyright (C) 2016-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CPU_THREAD_COMPONENT_H_
#define _CPU_THREAD_COMPONENT_H_

/* Genode includes */
#include <base/thread.h>
#include <cpu_session/cpu_session.h>
#include <cpu_thread/client.h>

#include "config.h"
#include "append_list.h"
#include "cpu_session_component.h"

extern "C" int delete_gdb_breakpoint_at(long long where);

namespace Gdb_monitor { class Cpu_thread_component; }

class Gdb_monitor::Cpu_thread_component : public Rpc_object<Cpu_thread>,
                                          public Append_list<Cpu_thread_component>::Element
{
	private:

		static constexpr bool  _verbose = false;

		Cpu_session_component &_cpu_session_component;
		Cpu_thread_client      _parent_cpu_thread;
		int const              _new_thread_pipe_write_end;
		int const              _breakpoint_len;
		unsigned char const   *_breakpoint_data;

		unsigned long          _lwpid { 0 };
		addr_t                 _initial_ip { 0 };

		/*
		 * SIGTRAP, SIGSTOP and SIGINT must get delivered to the gdbserver code
		 * in the same order that they were generated. Since these signals are
		 * generated by different threads, the signal ep is used as
		 * synchronization point.
		 */
		Signal_handler<Cpu_thread_component> _exception_handler;
		Signal_handler<Cpu_thread_component> _sigstop_handler;
		Signal_handler<Cpu_thread_component> _sigint_handler;

		int  _pipefd[2];
		bool _initial_sigtrap_pending = true;
		bool _initial_breakpoint_handled = false;

		/* data for breakpoint at first instruction */
		enum { MAX_BREAKPOINT_LEN = 8 }; /* value from mem-break.c */
		unsigned char _original_instructions[MAX_BREAKPOINT_LEN];
		addr_t _breakpoint_ip;

		bool _set_breakpoint_at_first_instruction(addr_t ip);
		void _remove_breakpoint_at_first_instruction();

		void _handle_exception();
		void _handle_sigstop();
		void _handle_sigint();

	public:

		Cpu_thread_component(Cpu_session_component   &cpu_session_component,
		                     Capability<Pd_session>   pd,
		                     Cpu_session::Name const &name,
		                     Affinity::Location       affinity,
		                     Cpu_session::Weight      weight,
		                     addr_t                   utcb,
		                     int const                new_thread_pipe_write_end,
		                     int const                breakpoint_len,
		                     unsigned char const     *breakpoint_data);

		~Cpu_thread_component();

		Signal_context_capability exception_signal_context_cap()
		{
			return _exception_handler;
		}

		Signal_context_capability sigstop_signal_context_cap()
		{
			return _sigstop_handler;
		}

		Signal_context_capability sigint_signal_context_cap()
		{
			return _sigint_handler;
		}

		Thread_capability thread_cap() { return cap(); }
		unsigned long lwpid() { return _lwpid; }

		Thread_capability parent_thread_cap()
		{
			return _parent_cpu_thread.rpc_cap();
		}

		int signal_pipe_read_fd() { return _pipefd[0]; }

		int handle_initial_breakpoint()
		{
			if (!_initial_breakpoint_handled) {
				_initial_breakpoint_handled = true;
				return 1;
			}

			return 0;
		}

		int send_signal(int signo);

		int deliver_signal(int signo);

		/**************************
		 ** CPU thread interface **
		 *************************/

		Dataspace_capability utcb() override;
		void start(addr_t, addr_t) override;
		void pause() override;
		void resume() override;
		void single_step(bool) override;
		Thread_state state() override;
		void state(Thread_state const &) override;
		void exception_sigh(Signal_context_capability) override;
		void affinity(Affinity::Location) override;
		unsigned trace_control_index() override;
		Dataspace_capability trace_buffer() override;
		Dataspace_capability trace_policy() override;
};

#endif /* _CPU_THREAD_COMPONENT_H_ */
