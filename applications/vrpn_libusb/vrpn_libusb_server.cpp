#include "vrpn_libusb.h"

#include <vrpn_Connection.h>

#include <math.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>

int main(int argc, char ** argv)
{
    std::string configFile = "vrpn.cfg";
    int port = 7701;
    int count = 1;

    while(count < argc)
    {
	std::string s = argv[count];

	if(s == "-f")
	{
	    count++;
	    if(count < argc)
	    {
		configFile = argv[count];
	    }
	}
	else if(s == "-p")
	{
	    count++;
	    if(count < argc)
	    {
		port = atoi(argv[count]);
	    }
	}
	else if(s == "-h" || s == "--help")
	{
	    std::cerr << "Usage: " << argv[0] << " [-p portNumber] [-f configFile]" << std::endl;
	    return 0;
	}

	count++;
    }

    std::cerr << "Starting server:" << std::endl;
    std::cerr << "Config File: " << configFile << std::endl;
    std::cerr << "Port: " << port << std::endl;
    std::cerr << "Device Name: Device0" << std::endl;

    std::stringstream address;
    address << ":" << port;

    vrpn_Connection * connection = vrpn_create_server_connection(address.str().c_str(),NULL,NULL);
    vrpn_libusb * lusb = new vrpn_libusb("Device0",connection,configFile);

    while(!lusb->isError())
    {
	lusb->mainloop();

	connection->mainloop();
    }

    delete lusb;
    delete connection;

    return 1;
}
