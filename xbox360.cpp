#include <boost/format.hpp>
#include <usb.h>
#include <unistd.h>
#include <iostream>

/*
Unknown data: bytes: 3 Data: 0x01 0x03 0x0e 
Unknown data: bytes: 3 Data: 0x02 0x03 0x00 
Unknown data: bytes: 3 Data: 0x03 0x03 0x03 
Unknown data: bytes: 3 Data: 0x08 0x03 0x00 

 */

struct XBox360Msg
{
  // --------------------------
  unsigned int dummy1      :8;
  unsigned int dummy2      :8;

  // data[2] ------------------
  unsigned int dpad_up     :1;
  unsigned int dpad_down   :1;
  unsigned int dpad_left   :1;
  unsigned int dpad_right  :1;

  unsigned int start       :1;
  unsigned int select      :1;

  unsigned int stick_left  :1;
  unsigned int stick_right :1;

  // data[3] ------------------
  unsigned int lb          :1;
  unsigned int rb          :1;
  unsigned int mode        :1;
  unsigned int dummy3      :1;

  unsigned int a           :1;
  unsigned int b           :1;
  unsigned int y           :1;
  unsigned int x           :1;

  // data[4] ------------------
  unsigned int lt          :8;
  unsigned int rt          :8;

  // data[6] ------------------
  int x1                   :16;
  int y1                   :16;

  // data[10] -----------------
  int x2                   :16;
  int y2                   :16;

  // data[14]; ----------------
  unsigned int dummy4      :32;
  unsigned int dummy5      :16;
} __attribute__((__packed__));

int get_bit(uint8_t data, int bit)
{
  return (data & (1 << bit)) >> bit;
}

struct usb_device* 
find_xbox360_controller()
{
  struct usb_bus* busses = usb_get_busses();

  for (struct usb_bus* bus = busses; bus; bus = bus->next) 
    {
      for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
        {
          if (0)
            std::cout << (boost::format("UsbDevice: idVendor: 0x%04x idProduct: 0x%04x")
                          % (int)dev->descriptor.idProduct 
                          % (int)dev->descriptor.idVendor)
                      << std::endl;

          if (dev->descriptor.idVendor  == 0x045e &&
              dev->descriptor.idProduct == 0x028e)
            return dev;
        }
    }
  return 0;
}

int main(int argc, char** argv)
{
  usb_init();
  usb_find_busses();
  usb_find_devices();
    
  struct usb_device* dev = find_xbox360_controller();
  if (!dev)
    {
      std::cout << "No XBox360 Controller found" << std::endl;
    }
  else 
    {
      std::cout << "XBox360 Controller found" << std::endl;
      struct usb_dev_handle* handle = usb_open(dev);
      if (!handle)
        {
          std::cout << "Error opening XBox360 controller" << std::endl;
        }
      else
        {
          if (1)
            {
              /* The LED-off command for Xbox-360 controllers */
              // Last byte is LED status:
              // 0: off
              // 1: all blinking
              // 2: top-left blink
              // 3: top-right blink
              // 4: bottom-left blink
              // 5: bottom-right blink
              // 6: top-left on
              // 7: top-right on
              // 8: bottom-left on
              // 9: bottom-right on
              // 10: rotate
              // 11: blink
              // 12: blink slower
              // 13: rotate with two lights
              // 14: blink
              // 15: blink once
              char ledcmd[] = {1, 3, 6}; 
              usb_bulk_write(handle, 2, ledcmd, 3, 0);
            }

          while(1)
            {
              uint8_t data[20];
              int ret = usb_bulk_read(handle, 1,
                                      (char*)data, 20, 0);
              XBox360Msg& msg = (XBox360Msg&)data;
              if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
                {
                  std::cout << boost::format("  S1:(%6d, %6d)") 
                    % int(msg.x1) % int(msg.y1);

                  std::cout << boost::format("  S2:(%6d, %6d)")
                    % int(msg.x2) % int(msg.y2);
                          
                  std::cout << boost::format(" [u:%d|d:%d|l:%d|r:%d]")
                    % int(msg.dpad_up)
                    % int(msg.dpad_down)
                    % int(msg.dpad_left)
                    % int(msg.dpad_right);

                  std::cout << "  select:" << msg.select;
                  std::cout << " mode:"    << msg.mode;
                  std::cout << " start:"   << msg.start;

                  std::cout << "  sl:" << msg.stick_left;
                  std::cout << " sr:"  << msg.stick_right;

                  std::cout << "  A:" << msg.a;
                  std::cout << " B:"  << msg.b;
                  std::cout << " X:"  << msg.x;
                  std::cout << " Y:"  << msg.y;

                  std::cout << "  LB:" << msg.lb;
                  std::cout << " RB:" <<  msg.rb;

                  std::cout << boost::format("  LT:%3d RT:%3d")
                    % int(msg.lt) % int(msg.rt);

                  std::cout << " Dummy: " << msg.dummy3 << " " << msg.dummy4 << " " << msg.dummy5 << std::endl;

                  std::cout << "\r" << std::flush;                         
                }
              else
                {
                  std::cout << "Unknown data: bytes: " << ret 
                            << " Data: ";
                      
                  for(int j = 0; j < ret; ++j)
                    {
                      std::cout << boost::format("0x%02x ") % int(data[j]);
                    }
                  //std::cout << "\r" << std::flush;
                  std::cout << std::endl;
                }
            }

          usb_close(handle);
        }
    }
  return 0;
}

/* EOF */