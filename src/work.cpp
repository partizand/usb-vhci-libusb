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
		work::work(uint8_t port) throw(std::invalid_argument) : port(port), canceled(false)
		{
			if(port == 0) throw std::invalid_argument("port");
		}

		work::~work() throw()
		{
		}

		void work::cancel() throw()
		{
			canceled = true;
		}

		process_urb_work::process_urb_work(uint8_t port, usb::urb* urb) throw(std::invalid_argument) :
			work(port),
			urb(urb)
		{
			if(!urb) throw std::invalid_argument("urb");
		}

		process_urb_work::process_urb_work(const process_urb_work& work) throw(std::bad_alloc) :
			usb::vhci::work(work),
			urb(new usb::urb(*work.urb))
		{
		}

		process_urb_work& process_urb_work::operator=(const process_urb_work& work) throw(std::bad_alloc)
		{
			usb::vhci::work::operator=(work);
			delete urb;
			urb = new usb::urb(*work.urb);
			return *this;
		}

		process_urb_work::~process_urb_work() throw()
		{
			delete urb;
		}

		cancel_urb_work::cancel_urb_work(uint8_t port, uint64_t handle) throw(std::invalid_argument) :
			work(port),
			handle(handle)
		{
		}

		port_stat_work::port_stat_work(uint8_t port, const port_stat& stat) throw(std::invalid_argument) :
			work(port),
			stat(stat),
			trigger_flags(0)
		{
		}

		port_stat_work::port_stat_work(uint8_t port,
		                               const port_stat& stat,
		                               const port_stat& prev) throw(std::invalid_argument) :
			work(port),
			stat(stat),
			trigger_flags(0)
		{
			if(!stat.get_enable() && prev.get_enable())     trigger_flags |= USB_VHCI_PORT_STAT_TRIGGER_DISABLE;
			if(stat.get_suspend() && !prev.get_suspend())   trigger_flags |= USB_VHCI_PORT_STAT_TRIGGER_SUSPEND;
			if(stat.get_resuming() && !prev.get_resuming()) trigger_flags |= USB_VHCI_PORT_STAT_TRIGGER_RESUMING;
			if(stat.get_reset() && !prev.get_reset())       trigger_flags |= USB_VHCI_PORT_STAT_TRIGGER_RESET;
			if(stat.get_power() && !prev.get_power())       trigger_flags |= USB_VHCI_PORT_STAT_TRIGGER_POWER_ON;
			else if(!stat.get_power() && prev.get_power())  trigger_flags |= USB_VHCI_PORT_STAT_TRIGGER_POWER_OFF;
		}
	}
}
