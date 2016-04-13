# -*- coding: utf-8 -*-

from __future__ import print_function
from __future__ import division

import sys
import shutil
from subprocess import check_output as ck_out
import json

from strideplatform import PlatformFunctions

# ---------------------
class Generator:
    def __init__(self, out_dir = '/home/andres/Documents/src/XMOS/Odo/StreamStack/platforms/Gamma/examples/filtering.stream_Products',
                 platform_dir = '/home/andres/Documents/src/XMOS/Odo/StreamStack/platforms/Gamma/1.0'):
        
    
        self.platform_dir = platform_dir
        self.out_dir = out_dir
        #out_dir = '/home/andres/Documents/src/XMOS/Odo/StreamStack/platforms/Gamma/1.0/test'
        #out_dir = sys.argv[1]
        self.project_dir = platform_dir + '/project'
        
        jsonfile = open(self.out_dir + '/tree.json')
        self.tree = json.load(jsonfile)
        
        self.platform = PlatformFunctions(platform_dir, self.tree)
        
        self.last_num_outs = 0
        
        # TODO get gamma sources and build and install if not available
        # Question: building requires cmake, should binaries be distributed instead?
        # What about secondary deps like portaudio and libsndfile?
        self.log("Building Gamma project")
        self.log("Buiding in directory: " + self.out_dir)
            
        shutil.copyfile(self.project_dir + "/template.cpp", self.out_dir + "/main.cpp")
        
    def log(self, text):
        print(text)
    
    def write_section_in_file(self, sec_name, code):
        filename = self.out_dir + "/main.cpp"
        f = open(filename, 'r')
        text = f.read()
        f.close()
    #    log(text)
        start_index = text.find("//[[%s]]"%sec_name)
        end_index = text.find("//[[/%s]]"%sec_name, start_index)
        if start_index <0 or end_index < 0:
            raise ValueError("Error finding [[%s]]  section"%sec_name)
            return
        text = text[:start_index] + '//[[%s]]\n'%sec_name + code + text[end_index:]
        f = open(filename, 'w')
        f.write(text)
        f.close()
    
    def generate_code(self):
        # Generate code from tree
        
        stream_index = 0;
        self.ugen_index = 0;
        self.includes_list = []
        
        config_template_code = '''
        AudioDevice adevi = AudioDevice::defaultInput();
        AudioDevice adevo = AudioDevice::defaultOutput();
        //AudioDevice adevi = AudioDevice("firewire_pcm");
        //AudioDevice adevo = AudioDevice("firewire_pcm");
        
        //int maxOChans = adevo.channelsOutMax();
        //int maxIChans = adevi.channelsOutMax();
        //printf("Max input channels:  %d\\n", maxIChans);
        //printf("Max output channels: %d\\n", maxOChans);
        
        // To open the maximum number of channels, we can hand in the queried values...
        //AudioIO io(256, 44100., audioCB, NULL, maxOChans, maxIChans);
        
        // ... or just use -1
        
        AudioIO io(%%block_size%%, %%sample_rate%%, audioCB, NULL, %%num_out_chnls%%, %%num_in_chnls%%);
        '''
        
        includes_code = ''
        declare_code = ''
        instantiation_code = ''
        init_code = ''
        processing_code = ''
        
        # TODO These defaults should be set from the platform definition file
        self.block_size = 512
        self.sample_rate = 44100.
        self.num_out_chnls = 2
        self.num_in_chnls = 2
        
    
        for node in self.tree:
            if 'stream' in node: # Everything grows from streams.
                code = self.platform.generate_stream_code(node["stream"], stream_index)
                declare_code += code["declare_code"]
                instantiation_code += code["instantiation_code"]
                init_code  += code["init_code"]
                processing_code += code["processing_code"]
                stream_index += 1
                
        #var_declaration = ''.join(['double stream_%02i;\n'%i for i in range(stream_index)])
        #declare_code = var_declaration + declare_code
        
        template_init_code = config_template_code
        template_init_code = template_init_code.replace("%%block_size%%", str(self.block_size))
        template_init_code = template_init_code.replace("%%sample_rate%%", str(self.sample_rate))
        template_init_code = template_init_code.replace("%%num_out_chnls%%", str(self.num_out_chnls))
        template_init_code = template_init_code.replace("%%num_in_chnls%%", str(self.num_in_chnls))
        
        init_code += template_init_code        
        
        includes_code = '\n'.join(set(self.includes_list)) + '\n'
        self.write_section_in_file('Includes', includes_code)
        self.write_section_in_file('Init Code', declare_code + instantiation_code)
        self.write_section_in_file('Config Code', init_code)
        self.write_section_in_file('Dsp Code', processing_code)
        
# Compile --------------------------
    def compile(self):
        import platform
        
        if platform.system() == "Windows":
            cpp_compiler = "/usr/bin/c++"
        
            flags = "-I"+ self.platform_dir +"/include -O3 -DNDEBUG -o " \
                + self.out_dir +"/main.cpp.o -c "+ self.out_dir +"/main.cpp"
            args = [cpp_compiler] + flags.split()
            try:
                outtext = ck_out(args)
            except:
                pass
        
            self.log(outtext)
        
            # Link ------------------------
            flags = "-O3 -DNDEBUG "+ self.out_dir + "/main.cpp.o -o " \
                + self.out_dir +"/app -rdynamic -L " \
                + self.platform_dir + "/lib -lGamma -lpthread -lportaudio -lsndfile -lpthread -lportaudio -lsndfile"
            args = [cpp_compiler] + flags.split()
            try:
                outtext = ck_out(args)
            except:
                pass
        
            self.log(outtext)
            self.log("Done.")
        
        
        elif platform.system() == "Linux":
            cpp_compiler = "/usr/bin/c++"
        
            flags = "-I"+ self.platform_dir +"/include -O3 -DNDEBUG -o" \
                + self.out_dir +"/main.cpp.o -c "+ self.out_dir + "/main.cpp"
            args = [cpp_compiler] + flags.split()
            
            #self.log(' '.join(args))
            #os.system(' '.join(args))
            outtext = ck_out(args)
        
            self.log(outtext)
        
            # Link ------------------------
            flags = "-O3 -DNDEBUG "+ self.out_dir +"/main.cpp.o -o"+ self.out_dir +"/app -rdynamic -L" \
                + self.platform_dir + "/lib -lGamma -lpthread -lportaudio -lsndfile -lpthread -lportaudio -lsndfile"
            args = [cpp_compiler] + flags.split()
            
            #self.log(' '.join(args))
            outtext = ck_out(args)
        
            self.log(outtext)
            self.log("Done.")
        
        elif platform.system() == "Darwin":
            cpp_compiler = "/usr/bin/c++"
        
            flags = "-I"+ self.platform_dir +"/include -O3 -DNDEBUG -o " \
                + self.out_dir +"/main.cpp.o -c "+ self.out_dir +"/main.cpp"
            args = [cpp_compiler] + flags.split()
            try:
                outtext = ck_out(args)
            except:
                pass
        
            self.log(outtext)
        
            # Link ------------------------
            flags = "-O3 -DNDEBUG "+ self.out_dir +"/main.cpp.o -o "+ self.out_dir +"/app -rdynamic -L " \
                + self.platform_dir + "/lib -lGamma -lpthread -lportaudio -lsndfile -lpthread -lportaudio -lsndfile"
            args = [cpp_compiler] + flags.split()
            try:
                outtext = ck_out(args)
            except:
                pass
        
            self.log(outtext)
            self.log("Done.")
        else:
            self.log("Platform '%s' not supported!"%platform.system())

   
if __name__ == '__main__':
    if len(sys.argv) < 2:
        gen = Generator('/home/andres/Documents/src/XMOS/Odo/StreamStack/platforms/Gamma/examples/passthru.stride_Products')
    else:
        gen = Generator(sys.argv[1])
    gen.generate_code()
#    gen.compile()
