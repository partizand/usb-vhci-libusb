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

#ifndef __LIBUSB_VHCI_H__
#define __LIBUSB_VHCI_H__

#include <stdint.h>

#ifdef __cplusplus
#include <errno.h>
#include <string>
#include <exception>
#include <stdexcept>
#endif

#define USB_VHCI_DEVICE_FILE "/dev/vhci-ctrl"

#ifdef __cplusplus
extern "C" {
#endif

struct usb_vhci_iso_packet
{
	uint32_t offset;
	int32_t packet_length, packet_actual;
	int32_t status;
};

struct usb_vhci_urb
{
	uint64_t handle;
	uint8_t *buffer;
	struct usb_vhci_iso_packet *iso_packets;
	int32_t buffer_length, buffer_actual;
	int32_t packet_count, error_count;
	int32_t status, interval;
	uint16_t flags;
#define USB_VHCI_URB_FLAGS_SHORT_NOT_OK 0x0001
#define USB_VHCI_URB_FLAGS_ISO_ASAP     0x0002
#define USB_VHCI_URB_FLAGS_ZERO_PACKET  0x0040
	uint16_t wValue, wIndex, wLength;
	uint8_t bmRequestType, bRequest;
	uint8_t devadr, epadr;
	uint8_t type;
#define USB_VHCI_URB_TYPE_ISO     0
#define USB_VHCI_URB_TYPE_INT     1
#define USB_VHCI_URB_TYPE_CONTROL 2
#define USB_VHCI_URB_TYPE_BULK    3
};

struct usb_vhci_port_stat
{
	uint16_t status, change;
#define USB_VHCI_PORT_STAT_CONNECTION    0x0001
#define USB_VHCI_PORT_STAT_ENABLE        0x0002
#define USB_VHCI_PORT_STAT_SUSPEND       0x0004
#define USB_VHCI_PORT_STAT_OVERCURRENT   0x0008
#define USB_VHCI_PORT_STAT_RESET         0x0010
#define USB_VHCI_PORT_STAT_POWER         0x0100
#define USB_VHCI_PORT_STAT_LOW_SPEED     0x0200
#define USB_VHCI_PORT_STAT_HIGH_SPEED    0x0400
#define USB_VHCI_PORT_STAT_C_CONNECTION  0x0001
#define USB_VHCI_PORT_STAT_C_ENABLE      0x0002
#define USB_VHCI_PORT_STAT_C_SUSPEND     0x0004
#define USB_VHCI_PORT_STAT_C_OVERCURRENT 0x0008
#define USB_VHCI_PORT_STAT_C_RESET       0x0010
	uint8_t index, flags;
#define USB_VHCI_PORT_STAT_FLAG_RESUMING 0x01
};

struct usb_vhci_work
{
	union
	{
		uint64_t handle;
		struct usb_vhci_urb urb;
		struct usb_vhci_port_stat port_stat;
	} work;

	int type;
#define USB_VHCI_WORK_TYPE_PORT_STAT   0
#define USB_VHCI_WORK_TYPE_PROCESS_URB 1
#define USB_VHCI_WORK_TYPE_CANCEL_URB  2
};

int usb_vhci_open(uint8_t port_count,
                  int32_t *id,
                  int32_t *usb_busnum,
                  char    *bus_id);
int usb_vhci_close(int fd);
int usb_vhci_fetch_work(int fd, struct usb_vhci_work *work);
int usb_vhci_fetch_data(int fd, const struct usb_vhci_urb *urb);
int usb_vhci_giveback(int fd, const struct usb_vhci_urb *urb);
int usb_vhci_update_port_stat(int fd, struct usb_vhci_port_stat stat);

#ifdef __cplusplus
} // extern "C"
#endif

#define usb_vhci_is_out(epadr)    !((epadr) & 0x80)
#define usb_vhci_is_in(epadr)     !!((epadr) & 0x80)
#define usb_vhci_is_iso(type)     ((type) == USB_VHCI_URB_TYPE_ISO)
#define usb_vhci_is_int(type)     ((type) == USB_VHCI_URB_TYPE_INT)
#define usb_vhci_is_control(type) ((type) == USB_VHCI_URB_TYPE_CONTROL)
#define usb_vhci_is_bulk(type)    ((type) == USB_VHCI_URB_TYPE_BULK)

#ifdef __cplusplus
namespace usb
{
	enum urbType
	{
		utIsochronous = USB_VHCI_URB_TYPE_ISO,
		utInterrupt   = USB_VHCI_URB_TYPE_INT,
		utControl     = USB_VHCI_URB_TYPE_CONTROL,
		utBulk        = USB_VHCI_URB_TYPE_BULK
	};

	class urb
	{
	private:
		usb_vhci_urb _urb;

		void _cpy(const usb_vhci_urb& u) throw();
		void _chk() throw(std::invalid_argument);

	public:
		urb(const urb&) throw();
		urb(uint64_t handle,
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
		    uint16_t wLength) throw(std::invalid_argument);
		urb(const usb_vhci_urb& urb) throw(std::invalid_argument);
		urb(const usb_vhci_urb& urb, bool own) throw(std::invalid_argument);
		virtual ~urb() throw();
		urb& operator=(const urb&) throw();

		const usb_vhci_urb* getInternal() const throw() { return &_urb; }
		uint64_t getHandle() const throw() { return _urb.handle; }
		uint8_t* getBuffer() const throw() { return _urb.buffer; }
		uint32_t getIsoPacketOffset(int32_t index) const throw() { return _urb.iso_packets[index].offset; }
		int32_t getIsoPacketLength(int32_t index) const throw() { return _urb.iso_packets[index].packet_length; }
		int32_t getIsoPacketActual(int32_t index) const throw() { return _urb.iso_packets[index].packet_actual; }
		int32_t getIsoPacketStatus(int32_t index) const throw() { return _urb.iso_packets[index].status; }
		uint8_t* getIsoPacketBuffer(int32_t index) const throw() { return _urb.buffer + _urb.iso_packets[index].offset; }
		int32_t getBufferLength() const throw() { return _urb.buffer_length; }
		int32_t getBufferActual() const throw() { return _urb.buffer_actual; }
		int32_t getIsoPacketCount() const throw() { return _urb.packet_count; }
		int32_t getIsoErrorCount() const throw() { return _urb.error_count; }
		int32_t getStatus() const throw() { return _urb.status; }
		int32_t getInterval() const throw() { return _urb.interval; }
		uint16_t getFlags() const throw() { return _urb.flags; }
		uint16_t getWValue() const throw() { return _urb.wValue; }
		uint16_t getWIndex() const throw() { return _urb.wIndex; }
		uint16_t getWLength() const throw() { return _urb.wLength; }
		uint8_t getBmRequestType() const throw() { return _urb.bmRequestType; }
		uint8_t getBRequest() const throw() { return _urb.bRequest; }
		uint8_t getDeviceAddress() const throw() { return _urb.devadr; }
		uint8_t getEndpointAddress() const throw() { return _urb.epadr; }
		urbType getType() const throw() { return static_cast<urbType>(_urb.type); }
		bool isIn() const throw() { return _urb.epadr & 0x80; }
		bool isOut() const throw() { return !isIn(); }
		void setStatus(int32_t value) throw() { _urb.status = value; }
		void ack() throw() { setStatus(0); }
		void stall() throw() { setStatus(-EPIPE); }
		void setBufferActual(int32_t value) throw() { _urb.buffer_actual = value; }
		void setIsoErrorCount(int32_t value) throw() { _urb.error_count = value; }
		bool isShortNotOk() const throw() { return _urb.flags & USB_VHCI_URB_FLAGS_SHORT_NOT_OK; }
		bool isZeroPacket() const throw() { return _urb.flags & USB_VHCI_URB_FLAGS_ZERO_PACKET; }
	};

	namespace vhci
	{
		class portStat
		{
		private:
			uint16_t status;
			uint16_t change;
			uint8_t flags;

		public:
			portStat() throw() : status(0), change(0), flags(0) { }
			portStat(uint16_t status, uint16_t change, uint8_t flags) throw()
				: status(status), change(change), flags(flags) { };
			virtual ~portStat() throw();
			uint16_t getStatus() const throw() { return status; }
			uint16_t getChange() const throw() { return change; }
			uint8_t getFlags()   const throw() { return flags; }
			void setStatus(uint16_t value) throw() { status = value; }
			void setChange(uint16_t value) throw() { change = value; }
			void setFlags(uint8_t value)   throw() { flags = value; }
			bool getResuming() const throw() { return flags & USB_VHCI_PORT_STAT_FLAG_RESUMING; }
			void setResuming(bool value) throw()
			{ flags = (flags & ~USB_VHCI_PORT_STAT_FLAG_RESUMING) | (value ? USB_VHCI_PORT_STAT_FLAG_RESUMING : 0); }
			bool getConnection()  const throw() { return status & USB_VHCI_PORT_STAT_CONNECTION; }
			bool getEnable()      const throw() { return status & USB_VHCI_PORT_STAT_ENABLE; }
			bool getSuspend()     const throw() { return status & USB_VHCI_PORT_STAT_SUSPEND; }
			bool getOvercurrent() const throw() { return status & USB_VHCI_PORT_STAT_OVERCURRENT; }
			bool getReset()       const throw() { return status & USB_VHCI_PORT_STAT_RESET; }
			bool getPower()       const throw() { return status & USB_VHCI_PORT_STAT_POWER; }
			bool getLowSpeed()    const throw() { return status & USB_VHCI_PORT_STAT_LOW_SPEED; }
			bool getHighSpeed()   const throw() { return status & USB_VHCI_PORT_STAT_HIGH_SPEED; }
			void setConnection(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_CONNECTION) |  (value ? USB_VHCI_PORT_STAT_CONNECTION : 0); }
			void setEnable(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_ENABLE) |      (value ? USB_VHCI_PORT_STAT_ENABLE : 0); }
			void setSuspend(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_SUSPEND) |     (value ? USB_VHCI_PORT_STAT_SUSPEND : 0); }
			void setOvercurrent(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_OVERCURRENT) | (value ? USB_VHCI_PORT_STAT_OVERCURRENT : 0); }
			void setReset(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_RESET) |       (value ? USB_VHCI_PORT_STAT_RESET : 0); }
			void setPower(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_POWER) |       (value ? USB_VHCI_PORT_STAT_POWER: 0); }
			void setLowSpeed(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_LOW_SPEED) |   (value ? USB_VHCI_PORT_STAT_LOW_SPEED : 0); }
			void setHighSpeed(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_HIGH_SPEED) |  (value ? USB_VHCI_PORT_STAT_HIGH_SPEED : 0); }
			bool getConnectionChanged()  const throw() { return change & USB_VHCI_PORT_STAT_C_CONNECTION; }
			bool getEnableChanged()      const throw() { return change & USB_VHCI_PORT_STAT_C_ENABLE; }
			bool getSuspendChanged()     const throw() { return change & USB_VHCI_PORT_STAT_C_SUSPEND; }
			bool getOvercurrentChanged() const throw() { return change & USB_VHCI_PORT_STAT_C_OVERCURRENT; }
			bool getResetChanged()       const throw() { return change & USB_VHCI_PORT_STAT_C_RESET; }
			void setConnectionChanged(bool value) throw()
			{ change = (change & ~USB_VHCI_PORT_STAT_C_CONNECTION) |  (value ? USB_VHCI_PORT_STAT_C_CONNECTION : 0); }
			void setEnableChanged(bool value) throw()
			{ change = (change & ~USB_VHCI_PORT_STAT_C_ENABLE) |      (value ? USB_VHCI_PORT_STAT_C_ENABLE : 0); }
			void setSuspendChanged(bool value) throw()
			{ change = (change & ~USB_VHCI_PORT_STAT_C_SUSPEND) |     (value ? USB_VHCI_PORT_STAT_C_SUSPEND : 0); }
			void setOvercurrentChanged(bool value) throw()
			{ change = (change & ~USB_VHCI_PORT_STAT_C_OVERCURRENT) | (value ? USB_VHCI_PORT_STAT_C_OVERCURRENT : 0); }
			void setResetChanged(bool value) throw()
			{ change = (change & ~USB_VHCI_PORT_STAT_C_RESET) |       (value ? USB_VHCI_PORT_STAT_C_RESET : 0); }
		};

#define USB_VHCI_PORT_STAT_TRIGGER_DISABLE   0x01
#define USB_VHCI_PORT_STAT_TRIGGER_SUSPEND   0x02
#define USB_VHCI_PORT_STAT_TRIGGER_RESUMING  0x04
#define USB_VHCI_PORT_STAT_TRIGGER_RESET     0x08
#define USB_VHCI_PORT_STAT_TRIGGER_POWER_ON  0x10
#define USB_VHCI_PORT_STAT_TRIGGER_POWER_OFF 0x20

		class work
		{
		private:
			uint8_t port;
			bool canceled;

		protected:
			work(uint8_t port) throw(std::invalid_argument);

		public:
			virtual ~work() throw();
			uint8_t getPort() const throw() { return port; }
			bool isCanceled() const throw() { return canceled; }
			void cancel() throw();
		};

		class processUrbWork : work
		{
		private:
			usb::urb* urb;

		public:
			processUrbWork(uint8_t port, usb::urb* urb) throw(std::invalid_argument);
			processUrbWork(const processUrbWork&) throw();
			processUrbWork& operator=(const processUrbWork&) throw();
			virtual ~processUrbWork() throw();
			usb::urb* getUrb() const throw() { return urb; }
		};

		class cancelUrbWork : work
		{
		private:
			uint64_t handle;

		public:
			cancelUrbWork(uint8_t port, uint64_t handle) throw(std::invalid_argument);
			uint64_t getHandle() const throw() { return handle; }
		};

		class portStatWork : work
		{
		private:
			portStat stat;
			uint8_t triggerFlags;

		public:
			portStatWork(uint8_t port, const portStat& stat) throw(std::invalid_argument);
			portStatWork(uint8_t port, const portStat& stat, const portStat& prev) throw(std::invalid_argument);
			const portStat& getPortStat() const throw() { return stat; }
			uint8_t getTriggerFlags()     const throw() { return triggerFlags; }
			bool triggersDisable()  const throw() { return triggerFlags & USB_VHCI_PORT_STAT_TRIGGER_DISABLE; }
			bool triggersSuspend()  const throw() { return triggerFlags & USB_VHCI_PORT_STAT_TRIGGER_SUSPEND; }
			bool triggersResuming() const throw() { return triggerFlags & USB_VHCI_PORT_STAT_TRIGGER_RESUMING; }
			bool triggersReset()    const throw() { return triggerFlags & USB_VHCI_PORT_STAT_TRIGGER_RESET; }
			bool triggersPowerOn()  const throw() { return triggerFlags & USB_VHCI_PORT_STAT_TRIGGER_POWER_ON; }
			bool triggersPowerOff() const throw() { return triggerFlags & USB_VHCI_PORT_STAT_TRIGGER_POWER_OFF; }
		};

		class hcd
		{
		private:
			uint8_t port_count;

			hcd(const hcd&) throw();
			hcd& operator=(const hcd&) throw();

		public:
			explicit hcd(uint8_t ports) throw(std::invalid_argument);
			virtual ~hcd() throw();
		};

		class local_hcd : public hcd
		{
		public:
			explicit local_hcd(uint8_t ports) throw(std::invalid_argument);
		};
	}
}
#endif // __cplusplus

#endif // __LIBUSB_VHCI_H__

