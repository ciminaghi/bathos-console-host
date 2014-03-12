#include <SaleaeDeviceApi.h>

#include <memory>
#include <iostream>
#include <string>
#include <unistd.h>
#include <stdio.h>

void __stdcall OnConnect(U64 device_id, GenericInterface* device_interface,
			 void* user_data);
void __stdcall OnDisconnect(U64 device_id, void* user_data);
void __stdcall OnReadData(U64 device_id, U8* data, U32 data_length,
			  void* user_data);
void __stdcall OnWriteData(U64 device_id, U8* data, U32 data_length,
			   void* user_data );
void __stdcall OnError(U64 device_id, void* user_data);

LogicInterface* gDeviceInterface = NULL;

U64 gLogicId = 0;
U32 gSampleRateHz = 4000000;

int main( int argc, char *argv[] )
{
  DevicesManagerInterface::RegisterOnConnect( &OnConnect );
  DevicesManagerInterface::RegisterOnDisconnect( &OnDisconnect );
  DevicesManagerInterface::BeginConnect();

  std::cout << std::uppercase <<
    "Devices are currently set up to read and write at " << gSampleRateHz <<
    " Hz.  You can change this in the code." << std::endl;

  while (1) {
    sleep(1);
  }

  return 0;
}

void __stdcall OnConnect( U64 device_id, GenericInterface* device_interface, void* user_data )
{
  if( dynamic_cast<LogicInterface*>( device_interface ) != NULL )
    {
      std::cout << "A Logic device was connected (id=0x" << std::hex << device_id << std::dec << ")." << std::endl;

      gDeviceInterface = (LogicInterface*)device_interface;
      gLogicId = device_id;

      gDeviceInterface->RegisterOnReadData( &OnReadData );
      gDeviceInterface->RegisterOnWriteData( &OnWriteData );
      gDeviceInterface->RegisterOnError( &OnError );

      gDeviceInterface->SetSampleRateHz( gSampleRateHz );
      gDeviceInterface->ReadStart();
    }
}

void __stdcall OnDisconnect( U64 device_id, void* user_data )
{
  if( device_id == gLogicId )
    {
      std::cout << "A device was disconnected (id=0x" << std::hex << device_id << std::dec << ")." << std::endl;
      gDeviceInterface = NULL;
    }
}

static U8 prev_clock;
static U8 prev_data;
static U8 first_byte = 1;
static U8 mask = 1;
static U8 curr_byte;
static bool started = false;

#define DATA_MASK (1 << 0)
#define CLOCK_MASK (1 << 1)

void __stdcall OnReadData( U64 device_id, U8* data, U32 data_length, void* user_data )
{
  U32 i;

  setbuf(stdout,NULL);

  for (i = 0; i < data_length; i++) {
    U8 b = data[i];
    U8 dat = b & DATA_MASK;
    U8 clk = b & CLOCK_MASK;
    if (first_byte) {
      prev_clock = clk;
      prev_data = dat;
      first_byte = 0;
      continue;
    }
    if (!started) {
      /* Look for a start */
      /* which is a rising edge on data with high clock */
      if (!clk) {
	prev_clock = clk;
	prev_data = dat;
	continue;
      }
      if (dat & !prev_data) {
	started = 1;
	prev_clock = clk;
	prev_data = dat;
	continue;
      }
      continue;
    }
    if (!clk || prev_clock) {
      prev_clock = clk;
      continue;
    }
    //std::cout << "p" << std::endl;
    prev_clock = clk;
    if (dat)
      curr_byte |= mask;
    if (mask == 0x80) {
      std::cout << curr_byte ;
      mask = 1;
      curr_byte = 0;
      started = 0;
      continue;
    }
    mask <<= 1;
  }

  DevicesManagerInterface::DeleteU8ArrayPtr( data );
}

void __stdcall OnWriteData( U64 device_id, U8* data, U32 data_length, void* user_data )
{
  std::cerr << "ERROR" << std::endl;
}


void __stdcall OnError( U64 device_id, void* user_data )
{
  std::cout << "device error" << std::endl;
}
