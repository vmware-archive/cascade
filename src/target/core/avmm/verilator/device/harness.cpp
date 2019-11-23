#include "verilated.h"
#include "Vprogram_logic.h"

using namespace std;

namespace {
  
enum Request {
  READY = 0,
  READ,
  WRITE,
  WAIT,
  DONE
};

Vprogram_logic* pl_;
bool running_;
Request req_;

} // namespace

extern "C" void verilator_init() {
  pl_ = new Vprogram_logic();
  req_ = READY;
}

extern "C" void verilator_start() {
  for (running_ = true; running_; ) {
    switch (req_) {
      case READY:
        break;
      case READ:
        pl_->s0_read = 1;
        req_ = WAIT;
        break;
      case WRITE:
        pl_->s0_write = 1;
        req_ = WAIT;
        break;
      case WAIT:
        if (!pl_->s0_waitrequest) {
          req_ = DONE;
        }
        break;
      case DONE:
        pl_->s0_read = 0;
        pl_->s0_write = 0;
        req_ = READY;
        break;
    }
    pl_->eval();
    pl_->clk = !pl_->clk;
  }
  pl_->final();
  delete pl_;
}

extern "C" void verilator_stop() {
  running_ = false;
}

extern "C" void verilator_write(uint16_t addr, uint32_t val) {
  pl_->s0_address = addr;
  pl_->s0_writedata = val;
  for (req_ = WRITE; req_ != READY;);
}

extern "C" uint32_t verilator_read(uint16_t addr) {
  pl_->s0_address = addr;
  for (req_ = READ; req_ != READY;);
  return pl_->s0_readdata;
}
