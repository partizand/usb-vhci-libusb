/*
 * Copyright (C) 2009-2011 Michael Singer <michael@a-singer.de>
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

#include <algorithm>

#include "libusb_vhci.h"

namespace usb
{
	void urb::_cpy(const usb_vhci_urb& u) throw(std::bad_alloc)
	{
		if(_urb.buffer_length)
		{
			_urb.buffer = new uint8_t[_urb.buffer_length];
			std::copy(u.buffer, u.buffer + _urb.buffer_length, _urb.buffer);
		}
		if(_urb.packet_count)
		{
			_urb.iso_packets = new usb_vhci_iso_packet[_urb.packet_count];
			std::copy(u.iso_packets, u.iso_packets + _urb.packet_count, _urb.iso_packets);
		}
	}

	void urb::_chk() throw(std::invalid_argument)
	{
		switch(_urb.type)
		{
		case USB_VHCI_URB_TYPE_ISO:
			if(_urb.packet_count && !_urb.buffer_length)
				throw std::invalid_argument("urb");
			break;
		case USB_VHCI_URB_TYPE_INT:
		case USB_VHCI_URB_TYPE_CONTROL:
		case USB_VHCI_URB_TYPE_BULK:
			_urb.packet_count = 0;
			break;
		default:
			throw std::invalid_argument("urb");
		}
	}

	urb::urb(const urb& urb) throw(std::bad_alloc) : _urb(urb._urb)
	{
		_urb.buffer = NULL;
		_urb.iso_packets = NULL;
		_cpy(urb._urb);
	}

	urb::urb(uint64_t handle,
	         urb_type type,
	         int32_t buffer_length,
	         uint8_t* buffer,
	         bool own_buffer,
	         int32_t iso_packet_count,
	         usb_vhci_iso_packet* iso_packets,
	         bool own_iso_packets,
	         int32_t buffer_actual,
	         int32_t status,
	         int32_t error_count,
	         uint16_t flags,
	         uint16_t interval,
	         uint8_t devadr,
	         uint8_t epadr,
	         uint8_t bmRequestType,
	         uint8_t bRequest,
	         uint16_t wValue,
	         uint16_t wIndex,
	         uint16_t wLength) throw(std::invalid_argument, std::bad_alloc) : _urb()
	{
		_urb.handle = handle;
		_urb.buffer_length = buffer_length;
		_urb.buffer_actual = buffer_actual;
		_urb.status = status;
		_urb.flags = flags;
		_urb.interval = interval;
		_urb.devadr = devadr;
		_urb.epadr = epadr;
		if(type != urb_type_control)
		{
			if(bmRequestType) throw std::invalid_argument("bmRequestType");
			if(bRequest)      throw std::invalid_argument("bRequest");
			if(wValue)        throw std::invalid_argument("wValue");
			if(wIndex)        throw std::invalid_argument("wIndex");
			if(wLength)       throw std::invalid_argument("wLength");
		}
		if(type != urb_type_isochronous)
		{
			if(iso_packet_count) throw std::invalid_argument("iso_packet_count");
			if(iso_packets)     throw std::invalid_argument("iso_packets");
			if(error_count)     throw std::invalid_argument("error_count");
		}
		switch(type)
		{
		case urb_type_isochronous:
			_urb.type = USB_VHCI_URB_TYPE_ISO;
			_urb.packet_count = iso_packet_count;
			_urb.error_count = error_count;
			if(iso_packet_count)
			{
				if(!buffer_length) throw std::invalid_argument("iso_packet_count");
				if(iso_packets)
				{
					if(own_iso_packets)
					{
						_urb.iso_packets = iso_packets;
					}
					else
					{
						_urb.iso_packets = new usb_vhci_iso_packet[iso_packet_count];
						std::copy(iso_packets, iso_packets + iso_packet_count, _urb.iso_packets);
					}
				}
				else
				{
					throw std::invalid_argument("iso_packets");
				}
			}
			break;
		case urb_type_interrupt:
			_urb.type = USB_VHCI_URB_TYPE_INT;
			break;
		case urb_type_control:
			_urb.type = USB_VHCI_URB_TYPE_CONTROL;
			_urb.bmRequestType = bmRequestType;
			_urb.bRequest = bRequest;
			_urb.wValue = wValue;
			_urb.wIndex = wIndex;
			_urb.wLength = wLength;
			break;
		case urb_type_bulk:
			_urb.type = USB_VHCI_URB_TYPE_BULK;
			if(interval) throw std::invalid_argument("interval");
			break;
		default:
			throw std::invalid_argument("type");
		}
		if(buffer_length)
		{
			if(buffer)
			{
				if(own_buffer)
				{
					_urb.buffer = buffer;
				}
				else
				{
					_urb.buffer = new uint8_t[buffer_length];
					std::copy(buffer, buffer + buffer_length, _urb.buffer);
				}
			}
			else
			{
				_urb.buffer = new uint8_t[buffer_length];
			}
		}
	}

	urb::urb(const usb_vhci_urb& urb) throw(std::invalid_argument, std::bad_alloc) : _urb(urb)
	{
		_urb.buffer = NULL;
		_urb.iso_packets = NULL;
		_chk();
		_cpy(urb);
	}

	urb::urb(const usb_vhci_urb& urb, bool own) throw(std::invalid_argument, std::bad_alloc) : _urb(urb)
	{
		if(!own)
		{
			_urb.buffer = NULL;
			_urb.iso_packets = NULL;
			_chk();
			_cpy(urb);
		}
		else
		{
			_chk();
			if(!_urb.buffer_length && _urb.buffer)
				throw std::invalid_argument("urb");
			if(!_urb.packet_count && _urb.iso_packets)
				throw std::invalid_argument("urb");
		}
	}

	urb::~urb() throw()
	{
		if(_urb.buffer)
			delete[] _urb.buffer;
		if(_urb.iso_packets)
			delete[] _urb.iso_packets;
	}

	urb& urb::operator=(const urb& urb) throw(std::bad_alloc)
	{
		if(_urb.buffer)
			delete[] _urb.buffer;
		if(_urb.iso_packets)
			delete[] _urb.iso_packets;
		_urb = urb._urb;
		_urb.buffer = NULL;
		_urb.iso_packets = NULL;
		_cpy(urb._urb);
		return *this;
	}

	void urb::set_iso_results() throw(std::logic_error)
	{
		if(!is_isochronous())
			throw std::logic_error("not an isochronous urb");

		// count error statuses
		int32_t errors(0);
		for(int32_t i(0); i < get_iso_packet_count(); i++)
		{
			if(get_iso_packet_status(i) != USB_VHCI_STATUS_SUCCESS)
				errors++;
		}
		set_iso_error_count(errors);

		// set urb status according to packet statuses
		if(errors == get_iso_packet_count())
			set_status(USB_VHCI_STATUS_ALL_ISO_PACKETS_FAILED);
		else
			ack();

		// for IN isos: buffer_actual has to be equal to buffer_length
		if(is_in())
			set_buffer_actual(get_buffer_length());
	}
}
