
package.path = package.path .. ";/home/andres/Documents/src/XMOS/Odo/OdoEdit/lua_scripts/process_template.lua"
require 'process_template'
require 'parse_code'

print("Loaded process.lua")

local ugens = {}

function process(template_path)
   
   ugens = parse(code)

   process_template(template_path, ugens)
   print("Done process.")
end

process("/home/andres/StreamStack/untitled_03")
