#include <iostream>
#include <libusb_vhci.h>

int main()
{
	usb::vhci::local_hcd hcd(1);

	while(true)
	{
		usb::vhci::work* work;
		hcd.next_work(&work);
		if(work)
		{
			if(usb::vhci::port_stat_work* psw = dynamic_cast<usb::vhci::port_stat_work*>(work))
			{
				std::cout << "got port stat work" << std::endl;
			}
			hcd.finish_work(work);
		}
		else sleep(10);
	}

	return 0;
}

