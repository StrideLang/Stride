
#include "RtAudio.h"
#include <iostream>
#include <cstdlib>
#include <cstring>


//[[Includes]]
//[[/Includes]]


//[[Declarations]]
//[[/Declarations]]

//[[Instances]]
//[[/Instances]]


//[[Processing]]

int inout( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
           double streamTime, RtAudioStreamStatus status, void *data )
{
  // Since the number of input and output channels is equal, we can do
  // a simple buffer copy operation here.
  if ( status ) std::cout << "Stream over/underflow detected." << std::endl;
  unsigned long *bytes = (unsigned long *) data;
  memcpy( outputBuffer, inputBuffer, *bytes );
  return 0;
}
//[[/Processing]]

//[[OSC:Processing]]
//[[/OSC:Processing]]
//[[SerialIn:Processing]]
//[[/SerialIn:Processing]]
//[[SerialOut:Processing]]
//[[/SerialOut:Processing]]


int main() {
//[[Initialization]]

//[[/Initialization]]

// char input;
// std::cout << "\nRunning ... press <enter> to quit.\n";
// std::cin.get(input);

//[[Cleanup]]
//[[/Cleanup]]


 return 0;
}
