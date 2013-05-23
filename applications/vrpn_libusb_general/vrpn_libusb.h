#ifndef VRPN_LIBUSB_H

#include "vrpn_Button.h"
#include "vrpn_Analog.h"

#include <libusb-1.0/libusb.h>
#include <pthread.h>

#include <string>
#include <vector>

struct ThreadInit;

struct ButtonGroup
{
    unsigned int packetOffset;
    unsigned int numButtons;
    unsigned int value;
};

struct Valuator
{
    unsigned int packetOffset;
    int zeroMin;
    int zeroMax;
    int min;
    int max;
    float timeout;
    float value;
    bool isSigned;
};

class VRPN_API vrpn_libusb : public vrpn_Button, public vrpn_Analog
{
    public:
        vrpn_libusb(const char *name, vrpn_Connection *c, std::string configFile);
        virtual ~vrpn_libusb();

        bool isError();

        virtual void mainloop();

        int getNumButtons() { return num_buttons; }

        pthread_mutex_t _updateLock;
        
        std::vector<ButtonGroup*> _buttonGroups;
        std::vector<Valuator*> _valuators;

        int _packetSize;
        bool _printPacket;

        struct timeval _lastUpdateTime;
    protected:
        bool loadConfigFile(std::string & configFile);

        void printDevices(libusb_device ** list);
        libusb_device * findDevice(libusb_device ** list);
        void printConfig();

        libusb_context * _context;

        libusb_device ** _devList;
        libusb_device * _dev;
        libusb_device_handle * _handle;
        int _vendorID;
        int _productID;
        int _entryNum;
        int _configNum;
        int _numConfigs;
        int _interfaceNum;
        int _numInterfaces;
        int _altSettingNum;
        int _numAltSettings;
        int _endpointNum;
        int _numEndpoints;
        int _address;

        bool _error;

        pthread_t _updateThread;
        struct ThreadInit * _ti;
        unsigned char * _packet;
        bool _driverPresent;

        libusb_transfer * _transfer;
};

void transCallback(struct libusb_transfer * transfer);

void * libusbUpdate(void * init);

struct ThreadInit
{
    libusb_context * context;
    bool quit;
    pthread_mutex_t _quitLock;
};

#endif
