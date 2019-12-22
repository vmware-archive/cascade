#include <fcntl.h> 
#include <iostream>
#include <termios.h>
#include <unistd.h>

using namespace std;

int write_all(int fd, const char* data, size_t len) {
  //write(fd, data, len);
  for (size_t i = 0; i < len; i += write(fd, data+i, len-i)) {
    cout << "WROTE " << i << endl; 
  }
  return len;
}

int read_all(int fd, char* data, size_t len) {
  //read(fd, data, len);
  for (size_t i = 0; i < len; i += read(fd, data+i, len-i)) {
    cout << "READ " << i << endl;
  }
  return len;
}

int main() {
  const auto fd = open("/dev/cu.usbserial-120001", O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) {
    cout << "Unable to open device" << endl;
    return 1;
  }

  struct termios tty;
  const auto res1 = tcgetattr(fd, &tty);
  assert(res1 == 0);

  // Set baud rate for input and output
  cfsetospeed(&tty, B115200);
  cfsetispeed(&tty, B115200);

  // Send/receive 8-bit characters
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;   
  // Disable break processing
  tty.c_iflag &= ~IGNBRK;    
  // No signalling chars, echo, or cannonical processing
  tty.c_lflag = 0;
  // No remapping, no delayes
  tty.c_oflag = 0;                
  // Blocking reads
  tty.c_cc[VMIN] = 1; 
  // Shut off xon/xoff control
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); 
  // Ignore modem controls, enable reading
  tty.c_cflag |= (CLOCAL | CREAD);
  // Turn off parity
  tty.c_cflag &= ~(PARENB | PARODD);      
  tty.c_cflag |= 0;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  const auto res2 = tcsetattr(fd, TCSANOW, &tty);
  assert(res2 == 0);
  


  for (size_t i = 0; i < 10; ++i) {
    uint64_t val = (uint64_t(1) << 63) | (uint64_t(i) << 32) | uint64_t(i);
    write_all(fd, reinterpret_cast<char*>(&val), 8);
    uint32_t res = 0;
    read_all(fd, reinterpret_cast<char*>(&res), 4);
  }

  auto fail = false;
  for (size_t i = 0; i < 10; ++i) {
    uint64_t val = (uint64_t(0) << 63) | (uint64_t(i) << 32) | uint64_t(0);
    write_all(fd, reinterpret_cast<char*>(&val), 8);
    uint32_t res = 0;
    read_all(fd, reinterpret_cast<char*>(&res), 4);
    cout << res << endl;

    if (res != i << 1) {
      fail = true;
    }
  }

  close(fd);

  return fail ? 1 : 0;
}
