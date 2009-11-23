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

#include <pthread.h>
#include <stdint.h>

#ifdef __cplusplus
#include <errno.h>
#include <string>
#include <exception>
#include <stdexcept>
#include <vector>
#include <queue>
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
	enum urb_type
	{
		urb_type_isochronous = USB_VHCI_URB_TYPE_ISO,
		urb_type_interrupt   = USB_VHCI_URB_TYPE_INT,
		urb_type_control     = USB_VHCI_URB_TYPE_CONTROL,
		urb_type_bulk        = USB_VHCI_URB_TYPE_BULK
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
		    uint16_t wLength) throw(std::invalid_argument);
		urb(const usb_vhci_urb& urb) throw(std::invalid_argument);
		urb(const usb_vhci_urb& urb, bool own) throw(std::invalid_argument);
		virtual ~urb() throw();
		urb& operator=(const urb&) throw();

		const usb_vhci_urb* get_internal() const throw() { return &_urb; }
		uint64_t get_handle() const throw() { return _urb.handle; }
		uint8_t* get_buffer() const throw() { return _urb.buffer; }
		uint32_t get_iso_packet_offset(int32_t index) const throw() { return _urb.iso_packets[index].offset; }
		int32_t get_iso_packet_length(int32_t index) const throw() { return _urb.iso_packets[index].packet_length; }
		int32_t get_iso_packet_actual(int32_t index) const throw() { return _urb.iso_packets[index].packet_actual; }
		int32_t get_iso_packet_status(int32_t index) const throw() { return _urb.iso_packets[index].status; }
		uint8_t* get_iso_packet_buffer(int32_t index) const throw() { return _urb.buffer + _urb.iso_packets[index].offset; }
		int32_t get_buffer_length() const throw() { return _urb.buffer_length; }
		int32_t get_buffer_actual() const throw() { return _urb.buffer_actual; }
		int32_t get_iso_packet_count() const throw() { return _urb.packet_count; }
		int32_t get_iso_error_count() const throw() { return _urb.error_count; }
		int32_t get_status() const throw() { return _urb.status; }
		int32_t get_interval() const throw() { return _urb.interval; }
		uint16_t get_flags() const throw() { return _urb.flags; }
		uint16_t get_wValue() const throw() { return _urb.wValue; }
		uint16_t get_wIndex() const throw() { return _urb.wIndex; }
		uint16_t get_wLength() const throw() { return _urb.wLength; }
		uint8_t get_bmRequestType() const throw() { return _urb.bmRequestType; }
		uint8_t get_bRequest() const throw() { return _urb.bRequest; }
		uint8_t get_device_address() const throw() { return _urb.devadr; }
		uint8_t get_endpoint_address() const throw() { return _urb.epadr; }
		urb_type get_type() const throw() { return static_cast<urb_type>(_urb.type); }
		bool is_in() const throw() { return _urb.epadr & 0x80; }
		bool is_out() const throw() { return !is_in(); }
		void set_status(int32_t value) throw() { _urb.status = value; }
		void ack() throw() { set_status(0); }
		void stall() throw() { set_status(-EPIPE); }
		void set_buffer_actual(int32_t value) throw() { _urb.buffer_actual = value; }
		void set_iso_error_count(int32_t value) throw() { _urb.error_count = value; }
		bool is_short_not_ok() const throw() { return _urb.flags & USB_VHCI_URB_FLAGS_SHORT_NOT_OK; }
		bool is_zero_packet() const throw() { return _urb.flags & USB_VHCI_URB_FLAGS_ZERO_PACKET; }
	};

	namespace vhci
	{
		class port_stat
		{
		private:
			uint16_t status;
			uint16_t change;
			uint8_t flags;

		public:
			port_stat() throw() : status(0), change(0), flags(0) { }
			port_stat(uint16_t status, uint16_t change, uint8_t flags) throw()
				: status(status), change(change), flags(flags) { };
			virtual ~port_stat() throw();
			uint16_t get_status() const throw() { return status; }
			uint16_t get_change() const throw() { return change; }
			uint8_t get_flags()   const throw() { return flags; }
			void set_status(uint16_t value) throw() { status = value; }
			void set_change(uint16_t value) throw() { change = value; }
			void set_flags(uint8_t value)   throw() { flags = value; }
			bool get_resuming() const throw() { return flags & USB_VHCI_PORT_STAT_FLAG_RESUMING; }
			void set_resuming(bool value) throw()
			{ flags = (flags & ~USB_VHCI_PORT_STAT_FLAG_RESUMING) | (value ? USB_VHCI_PORT_STAT_FLAG_RESUMING : 0); }
			bool get_connection()  const throw() { return status & USB_VHCI_PORT_STAT_CONNECTION; }
			bool get_enable()      const throw() { return status & USB_VHCI_PORT_STAT_ENABLE; }
			bool get_suspend()     const throw() { return status & USB_VHCI_PORT_STAT_SUSPEND; }
			bool get_overcurrent() const throw() { return status & USB_VHCI_PORT_STAT_OVERCURRENT; }
			bool get_reset()       const throw() { return status & USB_VHCI_PORT_STAT_RESET; }
			bool get_power()       const throw() { return status & USB_VHCI_PORT_STAT_POWER; }
			bool get_low_speed()   const throw() { return status & USB_VHCI_PORT_STAT_LOW_SPEED; }
			bool get_high_speed()  const throw() { return status & USB_VHCI_PORT_STAT_HIGH_SPEED; }
			void set_connection(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_CONNECTION) |  (value ? USB_VHCI_PORT_STAT_CONNECTION : 0); }
			void set_enable(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_ENABLE) |      (value ? USB_VHCI_PORT_STAT_ENABLE : 0); }
			void set_suspend(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_SUSPEND) |     (value ? USB_VHCI_PORT_STAT_SUSPEND : 0); }
			void set_overcurrent(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_OVERCURRENT) | (value ? USB_VHCI_PORT_STAT_OVERCURRENT : 0); }
			void set_reset(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_RESET) |       (value ? USB_VHCI_PORT_STAT_RESET : 0); }
			void set_power(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_POWER) |       (value ? USB_VHCI_PORT_STAT_POWER: 0); }
			void set_low_speed(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_LOW_SPEED) |   (value ? USB_VHCI_PORT_STAT_LOW_SPEED : 0); }
			void set_high_speed(bool value) throw()
			{ status = (status & ~USB_VHCI_PORT_STAT_HIGH_SPEED) |  (value ? USB_VHCI_PORT_STAT_HIGH_SPEED : 0); }
			bool get_connection_changed()  const throw() { return change & USB_VHCI_PORT_STAT_C_CONNECTION; }
			bool get_enable_changed()      const throw() { return change & USB_VHCI_PORT_STAT_C_ENABLE; }
			bool get_suspend_changed()     const throw() { return change & USB_VHCI_PORT_STAT_C_SUSPEND; }
			bool get_overcurrent_changed() const throw() { return change & USB_VHCI_PORT_STAT_C_OVERCURRENT; }
			bool get_reset_changed()       const throw() { return change & USB_VHCI_PORT_STAT_C_RESET; }
			void set_connection_changed(bool value) throw()
			{ change = (change & ~USB_VHCI_PORT_STAT_C_CONNECTION) |  (value ? USB_VHCI_PORT_STAT_C_CONNECTION : 0); }
			void set_enable_changed(bool value) throw()
			{ change = (change & ~USB_VHCI_PORT_STAT_C_ENABLE) |      (value ? USB_VHCI_PORT_STAT_C_ENABLE : 0); }
			void set_suspend_changed(bool value) throw()
			{ change = (change & ~USB_VHCI_PORT_STAT_C_SUSPEND) |     (value ? USB_VHCI_PORT_STAT_C_SUSPEND : 0); }
			void set_overcurrent_changed(bool value) throw()
			{ change = (change & ~USB_VHCI_PORT_STAT_C_OVERCURRENT) | (value ? USB_VHCI_PORT_STAT_C_OVERCURRENT : 0); }
			void set_reset_changed(bool value) throw()
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
			uint8_t get_port() const throw() { return port; }
			bool is_canceled() const throw() { return canceled; }
			void cancel() throw();
		};

		class process_urb_work : public work
		{
		private:
			usb::urb* urb;

		public:
			process_urb_work(uint8_t port, usb::urb* urb) throw(std::invalid_argument);
			process_urb_work(const process_urb_work&) throw();
			process_urb_work& operator=(const process_urb_work&) throw();
			virtual ~process_urb_work() throw();
			usb::urb* get_urb() const throw() { return urb; }
		};

		class cancel_urb_work : public work
		{
		private:
			uint64_t handle;

		public:
			cancel_urb_work(uint8_t port, uint64_t handle) throw(std::invalid_argument);
			uint64_t get_handle() const throw() { return handle; }
		};

		class port_stat_work : public work
		{
		private:
			port_stat stat;
			uint8_t trigger_flags;

		public:
			port_stat_work(uint8_t port, const port_stat& stat) throw(std::invalid_argument);
			port_stat_work(uint8_t port, const port_stat& stat, const port_stat& prev) throw(std::invalid_argument);
			const port_stat& get_port_stat() const throw() { return stat; }
			uint8_t get_trigger_flags()     const throw() { return trigger_flags; }
			bool triggers_disable()  const throw() { return trigger_flags & USB_VHCI_PORT_STAT_TRIGGER_DISABLE; }
			bool triggers_suspend()  const throw() { return trigger_flags & USB_VHCI_PORT_STAT_TRIGGER_SUSPEND; }
			bool triggers_resuming() const throw() { return trigger_flags & USB_VHCI_PORT_STAT_TRIGGER_RESUMING; }
			bool triggers_reset()    const throw() { return trigger_flags & USB_VHCI_PORT_STAT_TRIGGER_RESET; }
			bool triggers_power_on()  const throw() { return trigger_flags & USB_VHCI_PORT_STAT_TRIGGER_POWER_ON; }
			bool triggers_power_off() const throw() { return trigger_flags & USB_VHCI_PORT_STAT_TRIGGER_POWER_OFF; }
		};

		class hcd
		{
		private:
			std::vector<void (*)(hcd&)> work_enqueued_callbacks;

			pthread_t bg_thread;
			volatile bool thread_shutdown;
			uint8_t port_count;
			pthread_mutex_t thread_sync;

			std::queue<work*> inbox;
			std::list<work*> processing;

			hcd(const hcd&) throw();
			hcd& operator=(const hcd&) throw();

		protected:
			explicit hcd(uint8_t ports) throw(std::invalid_argument);
			virtual void bg_work() throw() = 0;
			virtual uint8_t address_from_port(uint8_t port) const throw(std::exception) = 0;
			virtual uint8_t port_from_address(uint8_t address) const throw(std::exception) = 0;
			virtual void canceling_work(work* work, bool in_progress) throw(std::exception);
			virtual void finishing_work(work* work) throw(std::exception);

		public:
			virtual ~hcd() throw();

			void add_work_enqueued_callback(void (*callback)(hcd&)) throw();
			void remove_work_enqueued_callback(void (*callback)(hcd&)) throw();
			virtual port_stat get_port_stat(uint8_t port) const throw(std::exception) = 0;
			virtual void port_connect(uint8_t port, usb::data_rate rate) throw(std::exception) = 0;
			virtual void port_disconnect(uint8_t port) throw(std::exception) = 0;
			virtual void port_disable(uint8_t port) throw(std::exception) = 0;
			virtual void port_resumed(uint8_t port) throw(std::exception) = 0;
			virtual void port_overcurrent(uint8_t port, bool set) throw(std::exception) = 0;
			virtual void port_reset_done(uint8_t port, bool enable) throw(std::exception) = 0;
			void port_reset_done(uint8_t port) throw(std::exception) { port_reset_done(port, true); }
			uint8_t get_port_count() const throw() { return port_count; }
			virtual work* next_work() throw(std::exception) = 0;
		};

		class local_hcd : public hcd
		{
		private:
			int fd;
			struct _port_info
			{
				uint8_t adr;
				port_stat stat;
				_port_info() throw() : adr(0), stat() { }
			}* port_info;

			local_hcd(const local_hcd&) throw();
			local_hcd& operator=(const local_hcd&) throw();

			uint8_t port_from_address(uint8_t adr) const throw(std::exception);

		public:
			explicit local_hcd(uint8_t ports) throw(std::invalid_argument, std::exception);
			virtual ~local_hcd() throw();

			virtual work* next_work() throw(std::exception);
		};
	}
}
#endif // __cplusplus

#endif // __LIBUSB_VHCI_H__

