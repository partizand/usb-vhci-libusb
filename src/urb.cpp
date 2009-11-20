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

#include <algorithm>

#include "libusb_vhci.h"

namespace usb
{
	void urb::_cpy(const usb_vhci_urb& u) throw()
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

	urb::urb(const urb& urb) throw() : _urb(urb._urb)
	{
		_urb.buffer = NULL;
		_urb.iso_packets = NULL;
		_cpy(urb._urb);
	}

	urb::urb(uint64_t handle,
	         urbType type,
	         int32_t bufferLength,
	         uint8_t* buffer,
	         bool ownBuffer,
	         int32_t isoPacketCount,
	         usb_vhci_iso_packet* isoPackets,
	         bool ownIsoPackets,
	         int32_t bufferActual,
	         int32_t status,
	         int32_t errorCount,
	         uint16_t flags,
	         uint16_t interval,
	         uint8_t devadr,
	         uint8_t epadr,
	         uint8_t bmRequestType,
	         uint8_t bRequest,
	         uint16_t wValue,
	         uint16_t wIndex,
	         uint16_t wLength) throw(std::invalid_argument) : _urb()
	{
		_urb.handle = handle;
		_urb.buffer_length = bufferLength;
		_urb.buffer_actual = bufferActual;
		_urb.status = status;
		_urb.flags = flags;
		_urb.interval = interval;
		_urb.devadr = devadr;
		_urb.epadr = epadr;
		if(type != utControl)
		{
			if(bmRequestType) throw std::invalid_argument("bmRequestType");
			if(bRequest)      throw std::invalid_argument("bRequest");
			if(wValue)        throw std::invalid_argument("wValue");
			if(wIndex)        throw std::invalid_argument("wIndex");
			if(wLength)       throw std::invalid_argument("wLength");
		}
		if(type != utIsochronous)
		{
			if(isoPacketCount) throw std::invalid_argument("isoPacketCount");
			if(isoPackets)     throw std::invalid_argument("isoPackets");
			if(errorCount)     throw std::invalid_argument("errorCount");
		}
		switch(type)
		{
		case utIsochronous:
			_urb.type = USB_VHCI_URB_TYPE_ISO;
			_urb.packet_count = isoPacketCount;
			_urb.error_count = errorCount;
			if(isoPacketCount)
			{
				if(!bufferLength) throw std::invalid_argument("isoPacketCount");
				if(isoPackets)
				{
					if(ownIsoPackets)
					{
						_urb.iso_packets = isoPackets;
					}
					else
					{
						_urb.iso_packets = new usb_vhci_iso_packet[isoPacketCount];
						std::copy(isoPackets, isoPackets + isoPacketCount, _urb.iso_packets);
					}
				}
				else
				{
					throw std::invalid_argument("isoPackets");
				}
			}
			break;
		case utInterrupt:
			_urb.type = USB_VHCI_URB_TYPE_INT;
			break;
		case utControl:
			_urb.type = USB_VHCI_URB_TYPE_CONTROL;
			_urb.bmRequestType = bmRequestType;
			_urb.bRequest = bRequest;
			_urb.wValue = wValue;
			_urb.wIndex = wIndex;
			_urb.wLength = wLength;
			break;
		case utBulk:
			_urb.type = USB_VHCI_URB_TYPE_BULK;
			if(interval) throw std::invalid_argument("interval");
			break;
		default:
			throw std::invalid_argument("type");
		}
		if(bufferLength)
		{
			if(buffer)
			{
				if(ownBuffer)
				{
					_urb.buffer = buffer;
				}
				else
				{
					_urb.buffer = new uint8_t[bufferLength];
					std::copy(buffer, buffer + bufferLength, _urb.buffer);
				}
			}
			else
			{
				_urb.buffer = new uint8_t[bufferLength];
			}
		}
	}

	urb::urb(const usb_vhci_urb& urb) throw(std::invalid_argument) : _urb(urb)
	{
		_urb.buffer = NULL;
		_urb.iso_packets = NULL;
		_chk();
		_cpy(urb);
	}

	urb::urb(const usb_vhci_urb& urb, bool own) throw(std::invalid_argument) : _urb(urb)
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
			delete [] _urb.buffer;
		if(_urb.iso_packets)
			delete [] _urb.iso_packets;
	}

	urb& urb::operator=(const urb& urb) throw()
	{
		if(_urb.buffer)
			delete [] _urb.buffer;
		if(_urb.iso_packets)
			delete [] _urb.iso_packets;
		_urb = urb._urb;
		_urb.buffer = NULL;
		_urb.iso_packets = NULL;
		_cpy(urb._urb);
		return *this;
	}
}
