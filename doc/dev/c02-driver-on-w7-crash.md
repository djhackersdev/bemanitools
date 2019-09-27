# Follow-up (14th August 2019)
After publishing this post-mortem, I got messaged by a user on sows who was able to shed some more light on this issue.
The user was experiencing the same symptoms on a Win7 setup: blue screen once the firmware was flashed to the C02 IO.
This user's solutions was to use the USB2 ports on the PC instead of the USB3 ones. This is good to know and kinda
aligns with the weird things happening in the driver (see below).

# Post-mortem: C02 IO kernel module crash on Windows 7 (29th July 2019) by icex2
## Background
The original ezusbsys.sys kernel module, which is required to run the C02 IO, was compiled for Windows XP 32-bit, only.
There is a newer driver by Cypress, cyusb3.sys, which could be used to IO2 boards on newer Windows platforms, but does
not work with the C02 IO in combination with Konami's propriatery firmware. Thus, it was not possible to run the C02 IO
on anything than Windows XP 32-bit. But, with newer IIDX games running on Windows 7 64-bit, the C02 IO wasn't usable
anymore. Leaving aside, that the newer games actually require a BIO2 board and do not support C02 nor IO2 boards
anymore.

## The goal
I still wanted to use my cabinet with a C02 board on newer games which is possible with BT5 adding an emulation layer
and an interface (iidxio). This IO interface can be used to implement a driver that talks to a real IO again. Thus,
implementing a ezusb iidxio driver library, we can run newer games with an C02 IO as well.

However, there was no ezusbsys.sys driver that works on newer platforms required to run the newer games. But, Cypress
was nice and included the source code of the ezusbsys kernel module. With a few tweaks and a very recent version of
visual studio, it was quite easy to build this driver for newer platforms, including Windows 7, 8 and 10 in both
32-bit and 64-bit variants.

## The problem
But, when using this driver on certain combinations of newer hardware (max. 1-2 years old) and Windows 7, the kernel
module might crash after the Konami C02 firmware got flashed to the ezusb board. The result was a bluescreen and reboot.

However, the hardware was fine and the kernel module worked fine on another piece of hardware, the stock PC that was 
used with iidx 20 to 24. However, this hardware is not powerful enough to run iidx 25 and newer without stuttering
issues.

## The analysis/debugging
Note: The full source code can be found in the bemanitools-supplement package.

Setup:
* Native hardware with Windows 7 that was crashing
* Vmware with Windows 10 and Visual Studio 2019 to compile the kernel module. Target platform Windows 7 64-bit
* Booting Windows 7 in test mode to allow unsigned kernel modules to run and with debug output turned on
* dbgview on Windows 7 machine to get local kernel dbg output

Because I wanted to stick to Windows 7 in the beginning (refer to the solution section), I started debugging the kernel
module by enabling the debug message output that was already available in the code. However, since kernel debug message
printing can be very delayed, the kernel could not print various messages before the kernel crashed.

Thus, I started stripping the kernel module step by step to narrow down the possible spots causing the crash. After a
few hours, I got the (first) issue tracked down:

After the firmware was flashed, the device had to re-enumerate. When this happens, the function *Ezusb_PnPAddDevice*
is called to create a new instance of the device. Since this kernel module is acting as a filter driver, it has to
trap this call, and add a filter device before the real device in the device stack. Thus, each call to the ezusb device
hits the filter device first and the filter device calls the real device after doing some magic.

*Ezusb_PnPAddDevice* calls *Ezusb_CreateDeviceObject*. Afterwards, it checks the status of the call to 
*Ezusb_CreateDeviceObject* and if successful, it tries attaching the device to the device stack. However, instead of
using *IoAttachDeviceToDeviceStackSafe* it uses the unsafe variant *IoAttachDeviceToDeviceStack* which can lead to a
race condition on newer Windows Systems. Furthermore, all initialization of further variables of the *deviceObject*
needs to happen BEFORE doing that. Again, this is a race condition.

Next issue: Once the kernel calls *Ezusb_StartDevice* -> *Ezusb_ConfigureDevice* -> *Ezusb_SelectInterfaces*, it tries
to use *USBD_ParseConfigurationDescriptorEx* to get the interface from the configuration descriptor. However, that
fails for some unknown reason. I checked the data structure and it is perfectly fine and everything is there. Thus,
I wrote my own version *Ezusb_GetInterfaceFromConfigurationDescriptor* which does all the magic required to get this
part fixed:
```
PUSB_INTERFACE_DESCRIPTOR Ezusb_GetInterfaceFromConfigurationDescriptor(
	IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
	)
{
	if (!ConfigurationDescriptor) {
		Ezusb_KdPrint(("ERROR Ezusb_GetInterfaceFromConfigurationDescriptor NULL configuration desc"));
		return NULL;
	}

	if (ConfigurationDescriptor->wTotalLength < sizeof(USB_CONFIGURATION_DESCRIPTOR) + sizeof(USB_CONFIGURATION_DESCRIPTOR)) {
		Ezusb_KdPrint(("ERROR Ezusb_GetInterfaceFromConfigurationDescriptor configuration descriptor too small to have space for interface descriptor"));
		return NULL;
	}

	// hardcoding this to a single interface because we only care about the ezusb used with IIDX (C02 IO)
	if (ConfigurationDescriptor->bNumInterfaces < 1) {
		Ezusb_KdPrint(("ERROR Ezusb_GetInterfaceFromConfigurationDescriptor num interfaces 0"));
		return NULL;
	}

	// when retrieving the configuration descriptor from the usb device, the interface is located right next to it
	return (PUSB_INTERFACE_DESCRIPTOR) (((unsigned char*) ConfigurationDescriptor) + sizeof(USB_CONFIGURATION_DESCRIPTOR));
}
```

And next issue is just up ahead: Following the above, we have to call *Ezusb_USBD_CreateConfigurationRequestEx* to
create a USB configuration request to set the interface we want to use. This is executed with a *Ezusb_CallUSBD* call
which sends request to the real hardware. However, this request always fails. The call *IoCallDriver* inside 
*Ezusb_CallUSBD* always returns an NTSTATUS code that is not documented anywhere (can't find the exact status code
anymore, but once you get it, try to find it in the header file).

At this point, I had to give up. I already wasted too many hours and this is clearly a dead end.

## The solution
Once I realized that I got stuck with Windows 7 and I didn't want to buy (more) new hardware, I gave Windows 10 a try.
Surprisingly, this solved all the issues and the kernel module runs fine. The C02 board is flashable without crashing
and works with newer IIDX games.