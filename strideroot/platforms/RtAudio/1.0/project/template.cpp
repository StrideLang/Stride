
#include "RtAudio.h"
#include <iostream>
#include <cstdlib>
#include <cstring>

#define NUM_IN_CHANNELS 2
#define NUM_OUT_CHANNELS 2


typedef float MY_TYPE;
#define FORMAT RTAUDIO_FLOAT32

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

int main() {
//[[Initialization]]
    RtAudio adac;
    if ( adac.getDeviceCount() < 1 ) {
        std::cout << std::endl << "No audio devices found!" << std::endl;
        exit( -1 );
    }
    // Set the same number of channels for both input and output.
    unsigned int bufferBytes;
    unsigned int bufferFrames = 512;
    int iDevice = 0;
    int oDevice = 0;
    unsigned int fs = 44100;


    RtAudio::StreamOptions options;
    //options.flags |= RTAUDIO_NONINTERLEAVED;

    RtAudio::StreamParameters iParams, oParams;
    iParams.deviceId = 0; // first available device
    iParams.nChannels = NUM_IN_CHANNELS;
    oParams.deviceId = 0; // first available device
    oParams.nChannels = NUM_OUT_CHANNELS;

    bufferBytes = bufferFrames * NUM_OUT_CHANNELS * sizeof( MY_TYPE );

    if ( iDevice == 0 )
      iParams.deviceId = adac.getDefaultInputDevice();
    if ( oDevice == 0 )
        oParams.deviceId = adac.getDefaultOutputDevice();
    try {
        adac.openStream( &oParams, &iParams, FORMAT, fs, &bufferFrames, &audio_buffer_process, (void *)&bufferBytes, &options);
    }
    catch ( RtAudioError& e ) {
        e.printMessage();
        exit( -1 );
    }
//[[/Initialization]]


     try {
       adac.startStream();
       char input;
       std::cout << "\nRunning ... press <enter> to quit.\n";
       std::cin.get(input);
       // Stop the stream.
       adac.stopStream();
     }
     catch ( RtAudioError& e ) {
       e.printMessage();
       goto cleanup;
     }

cleanup:
//[[Cleanup]]
     if ( adac.isStreamOpen() ) adac.closeStream();
//[[Cleanup]]


 return 0;
}
