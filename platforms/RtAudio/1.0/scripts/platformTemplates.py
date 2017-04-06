# -*- coding: utf-8 -*-
"""
Created on Sun Apr 17 11:28:56 2016

@author: andres
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
