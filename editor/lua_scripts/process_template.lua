require 'generator'

function process_template(path, ugen_graph)
   -- process_list is a table that describes the sections that need to be processed within each file
   local process_list = {}
   process_list["/src/compute.xc"] = {"Compute Process"}
   process_list["/src/control_server.h"] = {"Control Server Ugen Controls"}
   process_list["/src/control_server.xc"] = {"Control Server Ugen Init"}
   -- "Control Server Control Init", "Control Server Control Process", "Control Server Input Process"} Not implemented yet
   -- These function handle hardware control I/O

   process_list["/src/dsp_server.xc"] = {"DSP Server Ugen Init",
					 "DSP Server Process"}
   process_list["/src/main.xc"] = {"Main DSP includes",
				   "Main DSP"}
   process_list["/src/server_interface.h"] = {"Server Controls",
					      "Server Buffers"}
   
   for name, labels in pairs(process_list) do
      local filename = path .. name
      print("Processing:" .. filename)
      process_file_labels(filename, ugen_graph, labels, 0)
   end 
   -- Read from dsp file template
   local dsp_file = io.open(path .. "/src/dsp_0.xc", "r")
   if not dsp_file then
      print("file not found: " .. dsp_file .. "\n")
      error()
   end
   local dsp_xc_text = dsp_file:read("*all")
   dsp_file:close()
   -- Read DSP header file
   dsp_file = io.open(path .. "/src/dsp_0.h", "r")
   if not dsp_file then
      print("file not found: " .. path .. "/src/dsp_0.h" .. "\n")
      error()
   end
   local dsp_h_text = dsp_file:read("*all")
   dsp_file:close()
   -- Create additional dsp files
   for dsp_index=1,gen_calculate_threads(ugen_graph)-1 do
      dsp_file = io.open(path .. "/src/dsp_" .. dsp_index .. ".xc", "w")
      if not dsp_file then
        error()
      end
      dsp_file:write(string.gsub(dsp_xc_text, "dsp_0", "dsp_" .. dsp_index))
      dsp_file:close()

      dsp_file = io.open(path .. "/src/dsp_" .. dsp_index .. ".h", "w")
      if not dsp_file then
	 error()
      end
      dsp_file:write(string.gsub(dsp_h_text, "dsp_0", "dsp_" .. dsp_index))
      dsp_file:close()
   end

   -- Process dsp files
   for dsp_index=0,gen_calculate_threads(ugen_graph)-1 do
      local filename = path .. "/src/dsp_" .. dsp_index .. ".xc"
      print("Processing DSP:" .. filename)
      local labels = {"DSP Ugen Structs",
		      "DSP Init Ugens",
		      "DSP Process Ugens",
		      "DSP Pointer Swap"}
      process_file_labels(filename, ugen_graph, labels, dsp_index)
   end
end

function process_file_labels(filename, ugen_graph, labels, index)
   local file = io.open(filename, "r")
   if not file then
      print("file not found\n")
      error()
   end
   local outtxt = process_labels(file:read("*all"), ugen_graph, labels, index)

   file:close()
   local err
   file, err = io.open(filename, "w")
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
   
   print("replace: " .. full_text:sub(start_index,end_index))
   print("with: " .. section_text)
   local outtxt = full_text:sub(0,start_index) .. section_text .. full_text:sub(end_index)
   
   return outtxt
end

function process_labels(template_txt, ugen_graph, labels, index)
   local outtxt = template_txt

   for _, label in ipairs(labels) do
      local func_name = 'gen_' .. string.gsub(label, ' ', '_')
      print('Calling ' .. func_name)
      local new_text = _G[func_name](ugen_graph, index) -- Call function by string name
      outtxt = replace_section(outtxt, new_text, label)
   end

   return outtxt
end
