
package.path = package.path .. ";/home/andres/Documents/src/XMOS/Odo/OdoEdit/lua_scripts/process_template.lua"
require 'process_template'

print("Loaded process.lua")

local ugens = {{name = "biquad", tile = 0, dspcore = 0, id = 0, ugen_data = {size = 6}},
	       {name = "biquad", tile = 0, dspcore = 0, id = 1, ugen_data = {size = 6}},
	       {name = "biquad", tile = 0, dspcore = 1, id = 0, ugen_data = {size = 6}}
	    }

function process(path)
   process_template(path, ugens)
   print("Done process.")
end
-- process_template("/home/andres/StreamStack/untitled_00", ugens)