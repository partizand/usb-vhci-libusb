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

		processUrbWork::processUrbWork(uint8_t port, usb::urb* urb) throw(std::invalid_argument) : work(port), urb(urb)
		{
			if(urb == NULL) throw std::invalid_argument("urb");
		}

		cancelUrbWork::cancelUrbWork(uint8_t port, uint64_t handle) throw(std::invalid_argument) : work(port), handle(handle)
		{
		}

		portStatWork::portStatWork(uint8_t port, const portStat& stat) throw(std::invalid_argument)
			: work(port), stat(stat), triggerFlags(0)
		{
		}

		portStatWork::portStatWork(uint8_t port, const portStat& stat, const portStat& prev) throw(std::invalid_argument)
			: work(port), stat(stat), triggerFlags(0)
		{
			if(!stat.getEnable() && prev.getEnable())     triggerFlags |= USB_VHCI_PORT_STAT_TRIGGER_DISABLE;
			if(stat.getSuspend() && !prev.getSuspend())   triggerFlags |= USB_VHCI_PORT_STAT_TRIGGER_SUSPEND;
			if(stat.getResuming() && !prev.getResuming()) triggerFlags |= USB_VHCI_PORT_STAT_TRIGGER_RESUMING;
			if(stat.getReset() && !prev.getReset())       triggerFlags |= USB_VHCI_PORT_STAT_TRIGGER_RESET;
			if(stat.getPower() && !prev.getPower())       triggerFlags |= USB_VHCI_PORT_STAT_TRIGGER_POWER_ON;
			else if(!stat.getPower() && prev.getPower())  triggerFlags |= USB_VHCI_PORT_STAT_TRIGGER_POWER_OFF;
		}
	}
}
