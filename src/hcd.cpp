/*
 * Copyright (C) 2009 Michael Singer <michael@a-singer.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "libusb_vhci.h"

namespace usb
{
	namespace vhci
	{
		hcd::hcd(uint8_t ports) throw(std::invalid_argument, std::bad_alloc) :
			work_enqueued_callbacks(),
			bg_thread(),
			thread_shutdown(false),
			thread_sync(),
			port_count(ports),
			_lock(),
			inbox(),
			processing()
		{
			if(ports == 0) throw std::invalid_argument("ports");
			pthread_mutex_init(&thread_sync, NULL);
			pthread_mutex_init(&_lock, NULL);
		}

		hcd::~hcd() throw()
		{
			join_bg_thread();
			pthread_mutex_destroy(&_lock);
			pthread_mutex_destroy(&thread_sync);
		}

		void hcd::on_work_enqueued() throw()
		{
			for(std::vector<void *(hcd&) throw()>::const_iterator i(work_enqueued_callbacks.begin());
			    i < work_enqueued_callbacks.end();
			    i++)
			{
				(*i)(*this);
			}
		}

		void hcd::enqueue_work(work* work) throw(std::bad_alloc)
		{
			lock(_lock);
			inbox.push(work);
		}

		void hcd::init_bg_thread() throw(std::exception)
		{
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
			// TODO: set priority
			int res;
			pthread_t t();
			{
				lock(thread_sync);
				if(bg_thread != pthread_t())
					throw std::exception();
				res = pthread_create(&t, NULL, bg_thread_start, NULL);
				if(res) goto cleanup;
				bg_thread = t;
			}
		cleanup:
			pthread_attr_destroy(&attr);
			if(res) throw std::exception();
		}

		void hcd::join_bg_thread() throw()
		{
			lock(thread_sync);
			if(bg_thread == pthread_t())
				throw std::exception();
			thread_shutdown = true;
			pthread_join(&bg_thread);
			thread_shutdown = false;
			bg_thread = pthread_t();
		}
	}
}
