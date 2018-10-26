#ifndef LINUX_DEVICE
#define LINUX_DEVICE

#include <inttypes.h>
#include <HardwareSerial.h>

#define SERIAL_LINUX_RX 21
#define SERIAL_LINUX_TX 2
#define UART_LINUX_NUMBER 2
#define SERIAL_DEFAULT_BAUD 115200

class LinuxManager : public HardwareSerial
{
  private:
    bool _linuxInitialized = false;
    bool _ethernetConection = false;
    bool _dongleConnection = false;

  public:
    // LINUX (OrangePiZero)

    LinuxManager() : HardwareSerial(UART_LINUX_NUMBER)
    {
    }

    void init()
    {
        this->begin(SERIAL_DEFAULT_BAUD, SERIAL_8N1, SERIAL_LINUX_RX, SERIAL_LINUX_TX);
    }

    inline void setInitialized(bool value)
    {
        _linuxInitialized = value;
    }

    inline bool getInitialized()
    {
        return _linuxInitialized;
    }
};

#endif

extern LinuxManager Linux;