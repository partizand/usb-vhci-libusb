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

#include "vhci-hcd.h"
#include "libusb_vhci.h"

int usb_vhci_open(uint8_t port_count, int32_t *id, int32_t *usb_busnum, char *bus_id)
{
	int fd, err;
	struct vhci_ioc_register r;

	fd = open("/dev/vhci-ctrl", O_RDWR);
	if(fd == -1) return -1;

	memset(&r, 0, sizeof r);
	r.port_count = port_count;
	if(ioctl(fd, VHCI_HCD_IOCREGISTER, &r) == -1)
	{
		err = errno;
		usb_vhci_close(fd);
		errno = err;
		return -2;
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
	return TEMP_FAILURE_RETRY(close(fd));
}
