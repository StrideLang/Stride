
#include <stdio.h>
#include "Gamma/Gamma.h"
#include "Gamma/AudioIO.h"
#include "Gamma/Domain.h"

//[[Includes]]
//[[/Includes]]

using namespace gam;

//[[Init Code]]
float AudioOut[2];
float AudioIn[2];

        
struct Level {
    float Bypass;
float GainType;
float Output;
float Offset;
float Gain;
float Input;
void set_gainType(float value) {
GainType = value;

}
void set_gain(float value) {
Gain = value;

}
void set_offset(float value) {
Offset = value;

}
void set_bypass(float value) {
Bypass = value;

}
 Level() {
        GainType = 0;
Bypass = 0;
Input = 0;
Gain = 0;
Offset = 0;
Output = 0;

    }
    void process(float Input, float &Output) {
        // Starting stream 00 -------------------------
{
Bypass = GainType;

} // Stream End 00
// Starting stream 01 -------------------------
{
Output = ((Input * Gain) + Offset);

} // Stream End 01

    }
};
Level Level_001;
float _Level_001_out;
Level Level_007;
float _Level_007_out;
float Out[2];
float Test[2];
//[[/Init Code]]

void audioCB(AudioIOData & io)
{
    while(io()){
//[[Dsp Code]]
// Starting stream 00 -------------------------
{
AudioIn[0] = io.in(0);
AudioIn[1] = io.in(1);

Level_001.set_gain(0.1);Level_007.set_gain(0.1);Level_001.process(AudioIn[0], _Level_001_out);
Level_007.process(AudioIn[1], _Level_007_out);

io.out(0) = _Level_001_out;;
io.out(1) = _Level_007_out;;

} // Stream End 00
// Starting stream 01 -------------------------
{
Out[0] = Test[0];
Out[1] = Test[1];

} // Stream End 01
//[[/Dsp Code]]
    }
}


int main() {
//[[Config Code]]
AudioIn[0] = 0.0;
AudioIn[1] = 0.0;
AudioOut[0] = 0.0;
AudioOut[1] = 0.0;
Test[0] = 0.0;
Test[1] = 0.0;
Out[0] = 0.0;
Out[1] = 0.0;

    
    AudioDevice adevi = AudioDevice::defaultInput();
    AudioDevice adevo = AudioDevice::defaultOutput();
    
        adevi.print();
        adevo.print();
        
    
    AudioIO io(512, 44100, audioCB, NULL, 2, 2);
        //[[/Config Code]]
    io.deviceIn(adevi);
    io.deviceOut(adevo);

    Domain::master().spu(io.framesPerSecond());
    if(io.start()){
        printf("start successful\n");
        io.print();
        printf("\nPress 'enter' to quit...\n"); getchar();
    }
    else{
        printf("start failed\n");
        return -1;
    }
    return 0;
}
