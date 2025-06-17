#include <cli/cli.h>
#include <cli/clilocalsession.h>
#include <cli/loopscheduler.h>
#include <functional>
#include <thread>
#include "command_station.hpp"
#include "decoder.hpp"
#include "fifo.hpp"

namespace {

using namespace std::chrono_literals;

// Fake timer interrupt handler
std::function<void(uint32_t)> timer_irq_handler;

// Decoder task
void decoder_task() {
  Decoder decoder;

  // Initializing the decoder is mandatory
  decoder.init();

  // Register to timer interrupt
  timer_irq_handler = [&decoder](uint32_t ccr) { decoder.receive(ccr); };

  // Continuously call execute
  for (;;) {
    std::this_thread::sleep_for(5ms);
    decoder.execute();
  }
}

// Command station task
void command_station_task(FiFo<dcc::Packet>* fifo) {
  CommandStation command_station;

  // Initializing the command station is optional
  command_station.init({.num_preamble = DCC_TX_MIN_PREAMBLE_BITS,
                        .bit1_duration = 58u,
                        .bit0_duration = 100u,
                        .flags = {.bidi = true}});

  for (;;) {
    std::this_thread::sleep_for(1ms);

    // "Trigger" interrupt with latest timing
    auto const ccr{command_station.transmit()};
    timer_irq_handler(ccr);

    // Read packet from FIFO
    if (empty(*fifo)) continue;
    command_station.packet(fifo->front());
    fifo->pop_front();
  }
}

// REPL task reading user input
void repl_task(FiFo<dcc::Packet>* fifo) {
  // Start with primary address 3
  dcc::Address addr{3u, dcc::Address::BasicLoco};

  auto root{std::make_unique<cli::Menu>("dcc")};

  // Change address used in commands
  root->Insert("address",
               [&](std::ostream&, dcc::Address::value_type a) {
                 addr = a;
                 cli::Cli::cout() << "Set address to " << a << std::endl;
               },
               "Set address all commands are sent to",
               {"Address [0-16383] [default:3]"});

  // Set direction and speed
  root->Insert("direction_speed",
               [&](std::ostream&, bool dir, uint8_t speed) {
                 auto const packet{dcc::make_advanced_operations_speed_packet(
                   addr, dir << 7u | speed)};
                 fifo->push_back(packet);
               },
               "Set direction and speed",
               {"Direction [1 forward, 0 backward]", "Speed [0-127]"});

  // Set F4-F0
  root->Insert("f4-f0",
               [&](std::ostream&, uint8_t state) {
                 auto const packet{
                   dcc::make_function_group_f4_f0_packet(addr, state)};
                 fifo->push_back(packet);
               },
               "Functions F4-F0",
               {"State [0b00000-0b11111]"});

  // Set F8-F5
  root->Insert("f8-f5",
               [&](std::ostream&, uint8_t state) {
                 auto const packet{
                   dcc::make_function_group_f8_f5_packet(addr, state)};
                 fifo->push_back(packet);
               },
               "Functions F8-F5",
               {"State [0b00000-0b11111]"});

  // Read CV byte
  root->Insert("read_cv_byte",
               [&](std::ostream&, uint32_t cv_addr) {
                 auto const packet{
                   dcc::make_cv_access_long_verify_packet(addr, cv_addr)};
                 fifo->push_back(packet);
               },
               "Read CV byte",
               {"CV address [0-1023]"});

  // Write CV byte
  root->Insert("write_cv_byte",
               [&](std::ostream&, uint32_t cv_addr, uint8_t byte) {
                 auto const packet{
                   dcc::make_cv_access_long_write_packet(addr, cv_addr, byte)};
                 fifo->push_back(packet);
                 fifo->push_back(packet);
               },
               "Write CV byte",
               {"CV address [0-1023]", "CV value [0-255]"});

  // Read CV bit
  root->Insert("read_cv_bit",
               [&](std::ostream&, uint32_t cv_addr, bool bit, uint8_t pos) {
                 auto const packet{dcc::make_cv_access_long_verify_packet(
                   addr, cv_addr, bit, pos)};
                 fifo->push_back(packet);
               },
               "Read CV bit",
               {"CV address [0-1023]", "Bit", "Bit position [0-7]"});

  // Write CV bit
  root->Insert("write_cv_bit",
               [&](std::ostream&, uint32_t cv_addr, bool bit, uint8_t pos) {
                 auto const packet{dcc::make_cv_access_long_write_packet(
                   addr, cv_addr, bit, pos)};
                 fifo->push_back(packet);
                 fifo->push_back(packet);
               },
               "Write CV bit",
               {"CV address [0-1023]", "Bit", "Bit position [0-7]"});

  cli::Cli cli{std::move(root)};

  cli::LoopScheduler scheduler;
  cli::CliLocalTerminalSession session{cli, scheduler, std::cout};
  session.ExitAction([&scheduler](auto&&) { scheduler.Stop(); });
  scheduler.Run();
}

} // namespace

int main() {
  // Use a FIFO with internal locking to communicate between REPL and command
  // station
  FiFo<dcc::Packet> fifo;

  std::jthread decoder_thread{decoder_task};
  std::jthread command_station_thread{command_station_task, &fifo};
  std::jthread repl_thread{repl_task, &fifo};

  repl_thread.join();
  std::exit(0);
}
