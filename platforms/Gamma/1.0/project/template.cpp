
#include <stdio.h>
#include "Gamma/Gamma.h"
#include "Gamma/AudioIO.h"
#include "Gamma/Domain.h"

//[[Includes]]
//[[/Includes]]

using namespace gam;

//[[Declarations]]
//[[/Declarations]]

//[[Instances]]
//[[/Instances]]


//[[Processing]]

//[[/Processing]]

int main() {
//[[Initialization]]
    AudioDevice adevi = AudioDevice::defaultInput();
    AudioDevice adevo = AudioDevice::defaultOutput();
    //AudioDevice adevi = AudioDevice("firewire_pcm");
    //AudioDevice adevo = AudioDevice("firewire_pcm");

    //int maxOChans = adevo.channelsOutMax();
    //int maxIChans = adevi.channelsOutMax();
    //printf("Max input channels:  %d\n", maxIChans);
    //printf("Max output channels: %d\n", maxOChans);

    // To open the maximum number of channels, we can hand in the queried values...
    //AudioIO io(256, 44100., audioCB, NULL, maxOChans, maxIChans);

    // ... or just use -1

    AudioIO io(256, 44100., audioCB, NULL, -1, -1);
//[[/Initialization]]

    io.deviceIn(adevi);
    io.deviceOut(adevo);

    int returnval = 0;

    Domain::master().spu(io.framesPerSecond());
    if(io.start()){
        printf("start successful\n");
        io.print();
        printf("\nPress 'enter' to quit...\n"); getchar();
    }
    else{
        printf("start failed\n");
        returnval = -1;
        goto exit;
    }

exit:
//[[Cleanup]]
//[[Cleanup]]

    return returnval;
}
