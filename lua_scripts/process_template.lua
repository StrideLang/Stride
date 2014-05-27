require 'generator'

function process_template(path, ugens)
   local process_list = {}
   process_list["/src/compute.xc"] = {"Compute Process"}
   process_list["/src/control_server.h"] = {"Control Server Ugen Controls"}
   process_list["/src/control_server.xc"] = {"Control Server Ugen Init"}
   -- "Control Server Control Init", "Control Server Control Process", "Control Server Input Process"} Not implemented yet
   -- These function handle hardware control I/O
   process_list["/src/dsp_0.xc"] = {"DSP Ugen Structs",
				    "DSP Init Ugens",
				    "DSP Process Ugens",
				    "DSP Pointer Swap"}
   -- FIXME Need to figure out how to handle passing DSP thread index
   process_list["/src/dsp_server.xc"] = {"DSP Server Ugen Init",
					 "DSP Server Process"}
   process_list["/src/main.xc"] = {"Main DSP includes",
				   "Main DSP"}
   process_list["/src/server_interface.h"] = {"Server Controls",
					      "Server Buffers"}
   
   for name, labels in pairs(process_list) do
      local filename = path .. name
      print("Processing:" .. filename)
      process_file_labels(filename, ugens, labels, 0)
   end
end

function process_file_labels(filename, ugens, labels, index)
   local file = io.open(filename, "r")
   if not file then
      print("file not found\n")
      error()
   end
   local outtxt = process_labels(file:read("*all"), ugens, labels, index)

   file:close()
   local file, err = io.open(filename, "w")
   if not file then return print(err) end
   file:write(outtxt)
   file:close()
end

function replace_section(full_text, section_text, section_name)
   local start_index = full_text:find('//%[%[' .. section_name .. '%]%]')
   local end_index = full_text:find('//%[%[/' .. section_name .. '%]%]')

   if (not start_index) or (not end_index) then
      print("Error finding tag ".. section_name)
      error()
   end

   start_index = start_index + section_name:len() + 7
   
      local outtxt = full_text:sub(0,start_index) .. section_text .. full_text:sub(end_index)
   
   return outtxt
end

function process_labels(template_txt, ugens, labels, index)
   local outtxt = template_txt

   for i, label in ipairs(labels) do
      local func_name = 'gen_' .. string.gsub(label, ' ', '_')
      print('Calling ' .. func_name)
      local new_text = _G[func_name](ugens, index) -- Call function by string name
      outtxt = replace_section(outtxt, new_text, label)
   end

   return outtxt
end
