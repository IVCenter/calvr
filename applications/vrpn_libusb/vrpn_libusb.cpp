#include "vrpn_libusb.h"

#include <iostream>
#include <fstream>
#include <cstdio>

vrpn_libusb::vrpn_libusb(const char *name, vrpn_Connection *c, std::string configFile) : vrpn_Button(name,c), vrpn_Analog(name,c)
{
    _devList = NULL;
    _dev = NULL;
    _vendorID = -1;
    _productID = -1;
    _entryNum = 0;
    _configNum = 0;
    _numConfigs = 0;
    _interfaceNum = 0;
    _numInterfaces = 0;
    _altSettingNum = 0;
    _numAltSettings = 0;
    _endpointNum = 0;
    _numEndpoints = 0;
    _wheelOffset = 0;
    _withWheel = false;
    _wheelTimeout = 0.2;
    _address = 0;
    _packetSize = 0;
    _buttonOffsetSet = false;
    _buttonOffset = 0;
    _context = NULL;

    _error = false;

    num_buttons = 0;
    num_channel = 0;
    _buttonLocal = NULL;
    _valLocal = NULL;

    _driverPresent = false;

    if(!loadConfigFile(configFile))
    {
	_error = true;
	return;
    }

    if(libusb_init(&_context) < 0)
    {
	std::cerr << "Error libusb init." << std::endl;
	_error = true;
	return;
    }

    int devCount;

    if((devCount = libusb_get_device_list(NULL, &_devList)) < 0 )
    {
	std::cerr << "Error getting device list .." << std::endl;
	_error = true;
	return;	
    }

    if(_vendorID < 0 || _productID < 0)
    {
	std::cerr << "Error: vendorID/productID must be set." << std::endl;
	std::cerr << "Printing list of usb devices:" << std::endl;
	printDevices(_devList);
	_error = true;
	return;
    }

    bool usberror = false;

    _dev = findDevice(_devList);
    if(!_dev)
    {
	printConfig();
	std::cerr << "Error: device with vendorID/productID with given entry not found." << std::endl;
	std::cerr << "Printing list of usb devices:" << std::endl;
	printDevices(_devList);
	_error = true;
	return;
    }

    libusb_device_descriptor desc;
    if(libusb_get_device_descriptor(_dev,&desc) >= 0)
    {
	_numConfigs = (int)desc.bNumConfigurations;
	if(_configNum >= 0 && _configNum < _numConfigs)
	{
	    libusb_config_descriptor * cdesc;
	    if(libusb_get_config_descriptor(_dev,_configNum,&cdesc) >= 0)
	    {
		_numInterfaces = (int)cdesc->bNumInterfaces;
		if(_interfaceNum >= 0 && _interfaceNum < _numInterfaces)
		{
		    libusb_interface interface = cdesc->interface[_interfaceNum];
		    _numAltSettings = (int)interface.num_altsetting;
		    if(_altSettingNum >= 0 && _altSettingNum < _numAltSettings)
		    {
			libusb_interface_descriptor idesc = interface.altsetting[_altSettingNum];
			_numEndpoints = (int)idesc.bNumEndpoints;
			if(_endpointNum >= 0 && _endpointNum < _numEndpoints)
			{
			    libusb_endpoint_descriptor edesc = idesc.endpoint[_endpointNum];
			    _address = (int)edesc.bEndpointAddress;
			    _packetSize = (int)edesc.wMaxPacketSize;
			}
			else
			{
			    std::cerr << "Error: Invalid endpoint number." << std::endl;
			    usberror = true;
			}
		    }
		    else
		    {
			std::cerr << "Error: Invalid altSetting number." << std::endl;
			usberror = true;
		    }
		}
		else
		{
		    std::cerr << "Error: Invalid interface number." << std::endl;
		    usberror == true;
		}
	    }
	    else
	    {
		std::cerr << "Error: unable to get config descriptor." << std::endl;
		usberror = true;
	    }
	}
	else
	{
	    std::cerr << "Error: Invalid config number." << std::endl;
	    usberror = true;
	}
    }
    else
    {
	std::cerr << "Error: unable to get device descriptor." << std::endl;
	usberror = true;
    }

    printConfig();

    if(libusb_open(_dev, &_handle) < 0)
    {
	std::cerr << "Error opening device bus: " << libusb_get_bus_number(_dev) << " device: " << libusb_get_device_address(_dev) << std::endl;
	usberror = true;
    }

    if(usberror)
    {
	_error = true;
	return;
    }

    for(int i = 0; i < num_buttons; i++)
    {
	buttons[i] = 0;
	lastbuttons[i] = 0;
    }

    if(!num_buttons)
    {
	std::cerr << "Error: Number of Buttons is 0." << std::endl;
	_error = true;
    }
    else
    {
	_buttonLocal = new int[num_buttons];
	for(int i = 0; i < num_buttons; i++)
	{
	    _buttonLocal[i] = 0;
	}
    }

    if(_withWheel)
    {
	num_channel = 1;
	_valLocal = new float[1];
	_valLocal[0] = 0.0;
	channel[0] = 0.0;
	last[0] = 0.0;
    }

    _packet = new unsigned char[_packetSize];
    for(int i = 0; i < _numInterfaces; i++)
    {
	if(libusb_kernel_driver_active(_handle, i) == 1)
	{
	    _driverPresent = true;

	    if(libusb_detach_kernel_driver(_handle, i) != 0)
	    {
		std::cerr << "Error detaching driver from interface " << i << std::endl;
	    }
	}
	else
	{
	    _driverPresent = false;
	}
    }

    if(libusb_claim_interface(_handle, _interfaceNum) != 0)
    {
	std::cerr << "Error: could not claim device interface." << std::endl;
	_error = true;
	return;
    }

    _transfer = libusb_alloc_transfer(5);
    libusb_fill_interrupt_transfer(_transfer, _handle, _address, _packet, _packetSize, transCallback, (void*)this, 10000);
    libusb_submit_transfer(_transfer);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    _ti = new ThreadInit;
    _ti->context = _context;
    _ti->quit = false;
    pthread_create(&_updateThread,&attr,libusbUpdate,(void*)_ti);

    pthread_attr_destroy(&attr);

    std::cerr << "Init good." << std::endl;
}

vrpn_libusb::~vrpn_libusb()
{
    if(_devList)
    {
	libusb_free_device_list(_devList, 1);
    }
    if(_context)
    {
	libusb_exit(_context);
    }
}

bool vrpn_libusb::isError()
{
    return _error;
}

void vrpn_libusb::mainloop()
{
    struct timeval current_time;
    
    server_mainloop();

    vrpn_gettimeofday(&current_time, NULL);
    vrpn_Button::timestamp = current_time;
    vrpn_Analog::timestamp = current_time;

    pthread_mutex_lock(&_updateLock);

    if(_withWheel)
    {
	if(_valLocal[0] != 0.0)
	{
	    double timeDif = (current_time.tv_sec - _lastUpdateTime.tv_sec) + ((current_time.tv_usec - _lastUpdateTime.tv_usec) / 1000000.0);
	    //std::cerr << "Time diff: " << timeDif << std::endl;
	    if(timeDif > _wheelTimeout)
	    {
		_valLocal[0] = 0.0;
	    }
	}
	channel[0] = _valLocal[0];
    }

    for(int i = 0; i < num_buttons; i++)
    {
	buttons[i] = _buttonLocal[i];
    }

    pthread_mutex_unlock(&_updateLock);

    vrpn_Button::report_changes();
    vrpn_Analog::report_changes();

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 14000000;
    nanosleep(&ts,NULL);
}

bool vrpn_libusb::loadConfigFile(std::string & configFile)
{
    std::ifstream infile(configFile.c_str());
    std::string line;
    if(infile.is_open())
    {
	while(infile.good())
	{
	    std::getline(infile,line);
	    int value;
	    float fvalue;
	    if(sscanf(line.c_str(),"numbuttons %d",&value) == 1)
	    {
		num_buttons = value;
	    }
	    else if(sscanf(line.c_str(),"vendorid %d",&value) == 1)
	    {
		_vendorID = value;
	    }
	    else if(sscanf(line.c_str(),"productid %d",&value) == 1)
	    {
		_productID = value;
	    }
	    else if(sscanf(line.c_str(),"entry %d",&value) == 1)
	    {
		_entryNum = value;
	    }
	    else if(sscanf(line.c_str(),"config %d",&value) == 1)
	    {
		_configNum = value;
	    }
	    else if(sscanf(line.c_str(),"interface %d",&value) == 1)
	    {
		_interfaceNum = value;
	    }
	    else if(sscanf(line.c_str(),"altsetting %d",&value) == 1)
	    {
		_altSettingNum = value;
	    }
	    else if(sscanf(line.c_str(),"endpoint %d",&value) == 1)
	    {
		_endpointNum = value;
	    }
	    else if(sscanf(line.c_str(),"buttonoffset %d",&value) == 1)
	    {
		_buttonOffsetSet = true;
		_buttonOffset = value;
	    }
	    else if(sscanf(line.c_str(),"wheeloffset %d",&value) == 1)
	    {
		_withWheel = true;
		_wheelOffset = value;
	    }
	    else if(sscanf(line.c_str(),"wheeltimeout %f",&fvalue) == 1)
	    {
		_wheelTimeout = fvalue;
	    }
	}
    }
    else
    {
	std::cerr << "Unable to open config file: " << configFile << std::endl;
	return false;
    }

    return true;
}

void vrpn_libusb::printDevices(libusb_device ** list)
{
    libusb_device * dev;

    int i = 0;

    while((dev = list[i++]) != NULL)
    {
	libusb_device_descriptor desc;
	if(libusb_get_device_descriptor(dev, &desc) < 0)
	{
	    std::cerr << "Error getting device descriptor for usb device " << i-1 << std::endl;
	    continue;
	}

	libusb_device_handle * handle;
	if(libusb_open(dev, &handle) < 0)
	{
	    std::cerr << "Error opening device " << i-1 << std::endl;
	    continue;
	}

	unsigned char buffer[255];

	std::cerr << "Device: " << i-1 << std::endl;

	if(desc.iManufacturer)
	{
	    libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, buffer, 255);
	    std::cerr << "Manufacturer: " << buffer  << std::endl;
	}

	if(desc.iProduct)
	{
	    libusb_get_string_descriptor_ascii(handle, desc.iProduct, buffer, 255);
	    std::cerr << "Product: " << buffer  << std::endl;
	}

	if(desc.iSerialNumber)
	{
	    libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, buffer, 255);
	    std::cerr << "Serial Number: " << buffer  << std::endl;
	}

	std::cerr << "VendorID: " << desc.idVendor << std::endl;
	std::cerr << "ProductID: " << desc.idProduct << std::endl;
	std::cerr << std::endl;

	libusb_close(handle);
    } 
}

libusb_device * vrpn_libusb::findDevice(libusb_device ** list)
{
    libusb_device * dev;

    int i = 0;

    int entry = 0;

    while((dev = list[i++]) != NULL)
    {
	libusb_device_descriptor desc;
	if(libusb_get_device_descriptor(dev, &desc) < 0)
	{
	    //std::cerr << "Error getting device descriptor for usb device " << i-1 << std::endl;
	    continue;
	}


	if(_vendorID == desc.idVendor && _productID == desc.idProduct && entry >= _entryNum)
	{
	    return dev;
	}
	else if(_vendorID == desc.idVendor && _productID == desc.idProduct)
	{
	    entry++;
	}
    }
    return NULL;
}

void vrpn_libusb::printConfig()
{
    std::cerr << "Config Values:" << std::endl;
    std::cerr << "VendorID: " << _vendorID << std::endl;
    std::cerr << "ProductID: " << _productID << std::endl;
    std::cerr << "Entry Number: " << _entryNum << std::endl;
    std::cerr << "Config Number: " << _configNum << std::endl;
    std::cerr << "Number of Configs: " << _numConfigs << std::endl;
    std::cerr << "Interface Number: " << _interfaceNum << std::endl;
    std::cerr << "Number of Interfaces: " << _numInterfaces << std::endl;
    std::cerr << "Alt Setting Number: " << _altSettingNum << std::endl;
    std::cerr << "Number of Alt Settings: " << _numAltSettings << std::endl;
    std::cerr << "Endpoint Number: " << _endpointNum << std::endl;
    std::cerr << "Number of Endpoints: " << _numEndpoints << std::endl;
    std::cerr << "Endpoint Address: " << _address << std::endl;
    std::cerr << "Button Offset: " << _buttonOffset << std::endl;
    std::cerr << "Packet Size: " << _packetSize << std::endl;
    if(_withWheel)
    {
	std::cerr << "Wheel Offset: " << _wheelOffset << std::endl;
	std::cerr << "Wheel Timeout (sec): " << _wheelTimeout << std::endl;
    }
}

void transCallback(struct libusb_transfer * transfer)
{
    //std::cerr << "Callback." << std::endl;
    //std::cerr << "Status: " << transfer->status << std::endl;
    if(transfer->status == LIBUSB_TRANSFER_TIMED_OUT)
    {
        //std::cerr << "Timeout" << std::endl;
    }
    if(transfer->status == LIBUSB_TRANSFER_COMPLETED)
    {
	//std::cerr << "Got valid transfer. length: " << transfer->actual_length << std::endl;
	vrpn_libusb * mc = (vrpn_libusb*)transfer->user_data;

	if(!mc->_buttonOffsetSet)
	{
	    if(!transfer->actual_length)
	    {
		std::cerr << "No bytes read." << std::endl;
	    }
	    else
	    {
		std::cerr << "Packet: ";
		for(int i = 0; i < mc->_packetSize; i++)
		{
		    std::cerr << (int)(transfer->buffer)[i] << " ";
		}
		std::cerr << std::endl;
	    }
	}
	else if(transfer->actual_length)
	{
	    pthread_mutex_lock(&mc->_updateLock);
	    unsigned int mask = (unsigned int)(transfer->buffer)[mc->_buttonOffset];
            //std::cerr << "Button Mask: " << mask << std::endl;
	    unsigned int byte = 1;
	    for(int i = 0; i < mc->getNumButtons(); i++)
	    {
		if(mask & byte)
		{
		    mc->_buttonLocal[i] = 1;
		}
		else
		{
		    mc->_buttonLocal[i] = 0;
		}
		byte = byte << 1;
	    }

	    if(mc->_withWheel)
	    {
		float val = (float)((char)(transfer->buffer)[mc->_wheelOffset]);
		if(val > 1.0)
		{
		    val = 1.0;
		}
		else if(val < -1.0)
		{
		    val = -1.0;
		}
		mc->_valLocal[0] = val;
	    }
	    vrpn_gettimeofday(&mc->_lastUpdateTime,NULL);
	    pthread_mutex_unlock(&mc->_updateLock);
	}
	else
	{
	    //std::cerr << "No Bytes read." << std::endl;
	}
    }

    libusb_submit_transfer(transfer);
}

void * libusbUpdate(void * init)
{
    struct ThreadInit * ti = (ThreadInit*)init;
    while(1)
    {
	pthread_mutex_lock(&ti->_quitLock);
	if(ti->quit)
	{
	    pthread_mutex_unlock(&ti->_quitLock);
	    break;
	}
	pthread_mutex_unlock(&ti->_quitLock);
	libusb_handle_events(ti->context);
    }
    pthread_exit(NULL);
}
