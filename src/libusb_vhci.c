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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "vhci-hcd.h"
#include "libusb_vhci.h"

int usb_vhci_open(uint8_t port_count,  // [IN]  number of ports
                  int32_t *id,         // [OUT] controller id
                  int32_t *usb_busnum, // [OUT] usb bus number
                  char    *bus_id)     // [OUT] bus id (usually
                                       //       vhci_hcd.<controller id>)
{
	int fd = open(USB_VHCI_DEVICE_FILE, O_RDWR);
	if(fd == -1) return -1;

	struct vhci_ioc_register r;
	r.port_count = port_count;
	if(ioctl(fd, VHCI_HCD_IOCREGISTER, &r) == -1)
	{
		int err = errno;
		usb_vhci_close(fd);
		errno = err;
		return -1;
	}

	if(id) *id = r.id;
	if(usb_busnum) *usb_busnum = r.usb_busnum;
	if(bus_id)
	{
		size_t s = sizeof r.bus_id - sizeof *r.bus_id;
		memcpy(bus_id, r.bus_id, s);
		bus_id[s] = 0;
	}

	return fd;
}

int usb_vhci_close(int fd)
{
	int result;
	while((result = close(fd)) == -1 && errno == EINTR);
	return result;
}

int usb_vhci_fetch_work(int fd, struct usb_vhci_work *work)
{
	struct vhci_ioc_work w;
	if(ioctl(fd, VHCI_HCD_IOCFETCHWORK, &w) == -1)
		return -1;

	switch(w.type)
	{
	case VHCI_IOC_WORK_TYPE_PORT_STAT:
		work->type = USB_VHCI_WORK_TYPE_PORT_STAT;
		work->work.portstat.status = w.work.port.status;
		work->work.portstat.change = w.work.port.change;
		work->work.portstat.index  = w.work.port.index;
		work->work.portstat.flags  = w.work.port.flags;
		return 0;

	case VHCI_IOC_WORK_TYPE_PROCESS_URB:
		memset(&work->work.urb, 0, sizeof work->work.urb);
		switch(w.work.urb.type)
		{
		case VHCI_IOC_URB_TYPE_ISO:
			work->work.urb.type          = USB_VHCI_URB_TYPE_ISO;
			work->work.urb.packet_count  = w.work.urb.packet_count;
			work->work.urb.interval      = w.work.urb.interval;
			break;
		case VHCI_IOC_URB_TYPE_INT:
			work->work.urb.type          = USB_VHCI_URB_TYPE_INT;
			work->work.urb.interval      = w.work.urb.interval;
			break;
		case VHCI_IOC_URB_TYPE_CONTROL:
			work->work.urb.type          = USB_VHCI_URB_TYPE_CONTROL;
			work->work.urb.wValue        = w.work.urb.setup_packet.wValue;
			work->work.urb.wIndex        = w.work.urb.setup_packet.wIndex;
			work->work.urb.wLength       = w.work.urb.setup_packet.wLength;
			work->work.urb.bmRequestType = w.work.urb.setup_packet.bmRequestType;
			work->work.urb.bRequest      = w.work.urb.setup_packet.bRequest;
			break;
		case VHCI_IOC_URB_TYPE_BULK:
			work->work.urb.type          = USB_VHCI_URB_TYPE_BULK;
			work->work.urb.flags         = ((w.work.urb.flags & VHCI_IOC_URB_FLAGS_SHORT_NOT_OK) ?
			                                USB_VHCI_URB_FLAGS_SHORT_NOT_OK : 0) |
			                               ((w.work.urb.flags & VHCI_IOC_URB_FLAGS_ZERO_PACKET) ?
			                                USB_VHCI_URB_FLAGS_ZERO_PACKET : 0);
			break;
		default:
			errno = EBADMSG;
			return -1;<<
		}
		work->type = USB_VHCI_WORK_TYPE_PROCESS_URB;
		work->work.urb.status        = -EINPROGRESS;
		work->work.urb.handle        = w.handle;
		work->work.urb.buffer_length = w.work.urb.buffer_length;
		if(usb_vhci_is_out(w.work.urb.endpoint) || usb_vhci_is_iso(work->work.urb.type))
			work->work.urb.buffer_actual = w.work.urb.buffer_length;
		work->work.urb.devadr        = w.work.urb.address;
		work->work.urb.epadr         = w.work.urb.endpoint;
		// return 1 if usb_vhci_fetch_data should be called
		return work->work.urb.buffer_actual || work->work.urb.packet_count;

	case VHCI_IOC_WORK_TYPE_CANCEL_URB:
		work->type = USB_VHCI_WORK_TYPE_CANCEL_URB;
		work->work.handle = w.handle;
		return 0;

	default:
		errno = EBADMSG;
		return -1;
	}
}

int usb_vhci_fetch_data(int fd, const struct usb_vhci_urb *urb)
{
	struct vhci_ioc_urb_data u;
	u.handle        = urb->handle;
	u.buffer_length = urb->buffer_length;
	u.packet_count  = urb->packet_count;
	u.buffer        = urb->buffer;
	u.iso_packets   = urb->iso_packets;

	if(ioctl(fd, VHCI_HCD_IOCFETCHDATA, &u) == -1)
		return -1;

	return 0;
}
