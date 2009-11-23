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
		uint8_t local_hcd::getPortIndex(uint8_t adr) const throw(std::exception)
		{
			for(uint8_t i = 0; i < getPortCount(); i++)
				if(port_info[i].adr == adr)
					return i;
			throw std::exception();
		}

		local_hcd::local_hcd(uint8_t ports) throw(std::invalid_argument, std::exception) : hcd(ports), fd(-1), port_info(NULL)
		{
			fd = usb_vhci_open(ports, NULL, NULL, NULL);
			if(fd == -1) throw std::exception();
			port_info = new _port_info[ports];
		}

		local_hcd::~local_hcd() throw()
		{
			delete[] port_info;
			usb_vhci_close(fd);
		}

		work* local_hcd::nextWork() throw(std::exception)
		{
			usb_vhci_work w;
			int res = usb_vhci_fetch_work(fd, &w);
			if(res == -1)
			{
				if(errno == ETIMEDOUT || errno == EINTR || errno == ENODATA)
					return NULL;
				throw std::exception();
			}
			switch(w.type)
			{
			case USB_VHCI_WORK_TYPE_PORT_STAT:
			{
				portStat p(w.work.port_stat.status,
				           w.work.port_stat.change,
				           w.work.port_stat.flags);
				portStat prev = port_info[w.work.port_stat.index].stat;
				port_info[w.work.port_stat.index].stat = p;
				return new portStatWork(w.work.port_stat.index, p, prev);
			}
			case USB_VHCI_WORK_TYPE_PROCESS_URB:
			{
				if(w.work.urb.buffer_length)
					w.work.urb.buffer = new uint8_t[w.work.urb.buffer_length];
				if(w.work.urb.packet_count)
					w.work.urb.iso_packets = new usb_vhci_iso_packet[w.work.urb.packet_count];
				usb::urb* u(new usb::urb(w.work.urb, true));
				if(res)
				{
					res = usb_vhci_fetch_data(fd, u->getInternal());
					if(res == -1)
					{
						delete u;
						throw std::exception();
					}
				}
				return new processUrbWork(getPortIndex(w.work.urb.devadr), u);
			}
			case USB_VHCI_WORK_TYPE_CANCEL_URB:
				return new cancelUrbWork(0, w.work.handle);
			}
			throw std::exception();
		}
	}
}
