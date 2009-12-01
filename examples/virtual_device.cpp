#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include "../src/libusb_vhci.h"

bool has_work(true), waiting_for_work(false);
pthread_mutex_t has_work_mutex;
pthread_cond_t has_work_cv;

void signal_work_enqueued(void* arg, usb::vhci::hcd& from) throw()
{
	pthread_mutex_lock(&has_work_mutex);
	has_work = true;
	if(waiting_for_work)
	{
		waiting_for_work = false;
		pthread_cond_signal(&has_work_cv);
	}
	pthread_mutex_unlock(&has_work_mutex);
}

int main()
{
	pthread_mutex_init(&has_work_mutex, NULL);
	pthread_cond_init(&has_work_cv, NULL);

	usb::vhci::local_hcd hcd(1);
	std::cout << "created " << hcd.get_bus_id() << " (bus# " << hcd.get_usb_bus_num() << ")" << std::endl;
	hcd.add_work_enqueued_callback(usb::vhci::hcd::callback(&signal_work_enqueued, NULL));

	bool cont(false);
	while(true)
	{
		if(!cont)
		{
			pthread_mutex_lock(&has_work_mutex);
			if(!has_work)
			{
				waiting_for_work = true;
				pthread_cond_wait(&has_work_cv, &has_work_mutex);
			}
			else has_work = false;
			pthread_mutex_unlock(&has_work_mutex);
		}
		usb::vhci::work* work;
		cont = hcd.next_work(&work);
		if(work)
		{
			if(usb::vhci::port_stat_work* psw = dynamic_cast<usb::vhci::port_stat_work*>(work))
			{
				std::cout << "got port stat work" << std::endl;
				std::cout << "status: 0x" << std::setw(4) << std::setfill('0') <<
				             std::right << std::hex << psw->get_port_stat().get_status() << std::endl;
				std::cout << "change: 0x" << std::setw(4) << std::setfill('0') <<
				             std::right << psw->get_port_stat().get_change() << std::endl;
				std::cout << "flags:  0x" << std::setw(2) << std::setfill('0') <<
				             std::right << static_cast<int>(psw->get_port_stat().get_flags()) << std::endl;
				if(psw->get_port() != 1)
				{
					std::cerr << "invalid port" << std::endl;
					return 1;
				}
				if(psw->triggers_power_off())
				{
					std::cout << "port is powered off" << std::endl;
				}
				if(psw->triggers_power_on())
				{
					std::cout << "port is powered on -> connecting device" << std::endl;
					hcd.port_connect(1, usb::data_rate_full);
				}
				if(psw->triggers_reset())
				{
					std::cout << "port is resetting" << std::endl;
					if(hcd.get_port_stat(1).get_connection())
					{
						std::cout << "-> completing reset" << std::endl;
						hcd.port_reset_done(1);
					}
				}
				if(psw->triggers_resuming())
				{
					std::cout << "port is resuming" << std::endl;
					if(hcd.get_port_stat(1).get_connection())
					{
						std::cout << "-> completing resume" << std::endl;
						hcd.port_resumed(1);
					}
				}
				if(psw->triggers_suspend())
					std::cout << "port is suspended" << std::endl;
				if(psw->triggers_disable())
					std::cout << "port is disabled" << std::endl;
			}
			else if(usb::vhci::process_urb_work* puw = dynamic_cast<usb::vhci::process_urb_work*>(work))
			{
				std::cout << "got process urb work" << std::endl;

			}
			else if(usb::vhci::cancel_urb_work* cuw = dynamic_cast<usb::vhci::cancel_urb_work*>(work))
			{
				std::cout << "got cancel urb work" << std::endl;
			}
			else
			{
				std::cerr << "got invalid work" << std::endl;
				return 1;
			}
			hcd.finish_work(work);
		}
	}

	pthread_mutex_destroy(&has_work_mutex);
	pthread_cond_destroy(&has_work_cv);
	return 0;
}

