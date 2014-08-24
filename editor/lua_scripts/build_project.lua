
package.path = package.path .. ";/home/andres/Documents/src/XMOS/Odo/OdoEdit/lua_scripts/process_template.lua"
require 'process_template'
require 'parse_code'

print("Loaded process.lua")

local ugens = {}

function process(template_path, code)
   
   ugen_graph = parse(code)

   process_template(template_path, ugen_graph)
   print("Done process.")
end

-- process("/home/andres/StreamStack/untitled_03")
