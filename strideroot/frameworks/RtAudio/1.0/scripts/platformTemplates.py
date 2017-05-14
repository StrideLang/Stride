# -*- coding: utf-8 -*-
"""
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
"""

from BaseCTemplate import BaseCTemplate


class Templates(BaseCTemplate):
    def __init__(self, domain_rate = 44100):
        super(Templates, self).__init__(domain_rate)

        self.properties['num_in_channels'] = 2
        self.properties['num_out_channels'] = 2
        self.properties['audio_device'] = 0
        self.properties['sample_rate'] = 44100
        self.properties['block_size'] = 512

        self.framework = "RtAudio"

    def get_config_code(self):

        config_template_code = '''
    RtAudio adac;
    if ( adac.getDeviceCount() < 1 ) {
        std::cout << std::endl << "No audio devices found!" << std::endl;
        exit( -1 );
    }
    // Set the same number of channels for both input and output.
    unsigned int bufferBytes;
    unsigned int bufferFrames = %%block_size%%;
    int iDevice = 0;
    int oDevice = 0;
    unsigned int fs = %%sample_rate%%;


    RtAudio::StreamOptions options;
    //options.flags |= RTAUDIO_NONINTERLEAVED;

    RtAudio::StreamParameters iParams, oParams;
    iParams.deviceId = %%device%%; // first available device
    iParams.nChannels = %%num_in_chnls%%;
    oParams.deviceId = %%device%%; // first available device
    oParams.nChannels = %%num_out_chnls%%;

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
    '''

        config_template_code = config_template_code.replace("%%device%%", str(self.properties['audio_device']))
        config_template_code = config_template_code.replace("%%block_size%%", str(self.properties['block_size']))
        config_template_code = config_template_code.replace("%%sample_rate%%", str(self.properties['sample_rate']))
        config_template_code = config_template_code.replace("%%num_out_chnls%%", str(self.properties['num_out_channels']))
        config_template_code = config_template_code.replace("%%num_in_chnls%%", str(self.properties['num_in_channels']))

        return config_template_code

templates = Templates()
