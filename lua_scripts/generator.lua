
function gen_calculate_threads(ugens)
   local num_threads = table.getn(ugens)
   print("gen_calculate_threads: " .. num_threads)
   -- FIXME there needs to be a check to see if threads exceed the number available in hardware. If they do, then this needs to be handled properly.
   return num_threads
end

function gen_Compute_Process(ugens)
   local outstr = ''
   for i, chain in ipairs(ugens) do
      for j, ugen in ipairs(chain["ugens"]) do
         outstr = outstr ..
            string.format(
       [[
       case %s_%i_%02i_CONTROLS:
           ugen_%s_data_t * unsafe data = (ugen_%s_data_t *) ctr_data->data_buffer;
           unsafe {
           // TODO should check ctr->control_id to assign right one
           //ctr_data->%s_%i_%02i_controls.%s = ctr_data->value;
           }
           ugen_%s_ctl(ctr_data->%s_%i_%02i_controls, data);
           break;
     ]],
         string.upper(ugen["name"]),i - 1, ugen["id"],
         ugen["name"] ,ugen["name"],ugen["name"],
         i - 1, ugen["id"], "cf",
         ugen["name"], ugen["name"], i - 1, ugen["id"])
      end
   end
   return outstr
end

function gen_Control_Server_Ugen_Controls(ugens)
   local outstr = ''
   for i, chain in ipairs(ugens) do
      for j, ugen in ipairs(chain["ugens"]) do
      outstr = outstr .. string.format([[
        ugen_%s_controls_t %s_%i_%02i_controls;
]],
        ugen["name"], ugen["name"], i - 1, ugen["id"])
      end
   end
   return outstr
end

function gen_Control_Server_Ugen_Init(ugens)
   local part1 = ''
   local part2 = ''
   for i, chain in ipairs(ugens) do
      for j, ugen in ipairs(chain["ugens"]) do
         part1 = part1 .. string.format([[
         ugen_%s_init_controls(server_ctr_data.%s_%i_%02i_controls);
   ]], ugen["name"], ugen["name"], i - 1, ugen["id"])
     part2 = part2 .. string.format([[
       ugen_%s_init_data((ugen_%s_data_t *) buffers[%i].%s_%i_%02i_data);
      ]],
         ugen["name"], ugen["name"], i - 1,
         ugen["name"], i - 1, ugen["id"])
      end
   end
   local outstr = part1 .. "unsafe {\n" .. part2 .. "\n}\n"
   return outstr
end

function gen_Control_Server_Control_Process(ugens)
   local part1 = ''
   local part2 = ''
   for i, chain in ipairs(ugens) do
      for j, ugen in ipairs(chain["ugens"]) do
         part1 = part1 .. string.format([[
   case %s_%i_%02i_CONTROLS:
     datasize = sizeof(ugen_%s_data_t);
     break;
   ]], string.upper(ugen["name"]), i - 1, ugen["id"], ugen["name"])
         part2 = part2 .. string.format([[
     case %s_%i_%02i_CONTROLS:
             server_ctr_data.data_buffer =  buffers[%i].%s_%i_%02i_data;
        ctr_data = &server_ctr_data;
        server_ctr_data.value = button_val == BUTTON_DOWN ? 1:0;
        break;
      ]], string.upper(ugen["name"]), i - 1, ugen["id"], i - 1,
         ugen["name"], i - 1, ugen["id"])
      end   
   end
   local outstr = "switch(control_pending) {\n" .. part1 .. "}\n"
   outstr = outstr .. [[  memcpy(data.data, server_ctr_data.data_buffer, datasize);
  xchange_data = data;
  control_pending = -1;
  break;

  case comps.get_compute_data() -> control_data_t * unsafe ctr_data:
      server_ctr_data.index = control_pending;
      unsafe {
          switch(control_pending) {
          ]] .. part2 .. [[    }
      }
      break;
      ]]
      return outstr
end

function gen_DSP_Ugen_Structs(ugens, dsp_index)
   local outstr = ''
   for i, chain in ipairs(ugens) do
      for j, ugen in ipairs(chain["ugens"]) do
         if i - 1 == dsp_index then
       outstr = outstr .. string.format([[
         ugen_%s_state_t %s_%i_%02i;
   ]],
         ugen["name"], ugen["name"], i - 1, ugen["id"])
         end
      end
   end
   return outstr
end


function gen_DSP_Init_Ugens(ugens, dsp_index)
   outstr = ''
   for i, chain in ipairs(ugens) do
      for j, ugen in ipairs(chain["ugens"]) do
         if i - 1 == dsp_index then
       outstr = outstr .. string.format([[
           ugen_%s_init(%s_%i_%02i, (ugen_%s_data_t *) &(buffer.%s_%i_%02i_data[0]));
      ]],
         ugen["name"], ugen["name"], i - 1, ugen["id"],
         ugen["name"], ugen["name"], i - 1, ugen["id"])
         end
      end
   end
   return outstr
end

function gen_DSP_Process_Ugens(ugens, dsp_index)
   outstr = ''
   for i, ugen1 in ipairs(ugens) do
      -- TODO This is the tricky part!
   end
   return outstr
end

function gen_DSP_Pointer_Swap(ugens, dsp_index)
   local outstr = ''
   local part = ''
   for i, chain in ipairs(ugens) do
      for j, ugen in ipairs(chain["ugens"]) do
         if i - 1 == dsp_index then
       part = part .. string.format([[
                       case (%s_%i_%02i_CONTROLS:
                           (%s_%i_%02i.data = dsp_params.get_data((%s_%i_%02i.data);
                           break;
           ]],
           string.upper(ugen["name"]), i - 1, ugen["id"],
           ugen["name"], i - 1, ugen["id"],
           ugen["name"], i - 1, ugen["id"])
         end
      end
   end
   outstr = outstr .. [[select {
            case dsp_params.data_ready():
                ugen_id_t ugen_id = dsp_params.get_data_id();
                unsafe {
                    switch(ugen_id) {
]]
		 
   outstr = outstr .. part .. [[
                    }
                }
                break;
            default:
                break;
	}
]]
   return outstr
end


function gen_DSP_Server_Ugen_Init(ugens)
   local part1 = ''
   local part2 = ''
   local part3 = ''
   for i, chain in ipairs(ugens) do
      for j, ugen in ipairs(chain["ugens"]) do
         part1 = part1 .. string.format(
       "ugen_%s_data_t * unsafe %s_%i_%02i_data;\n",
       ugen["name"], ugen["name"], i - 1, ugen["id"])
         part2 = part2 .. string.format(
           "%s_%i_%02i_data = (ugen_%s_data_t *) buffers[%i].%s_%i_%02i_data;\n",
       ugen["name"], i - 1, ugen["id"],
       ugen["name"], i - 1,
       ugen["name"], i - 1, ugen["id"]
         )
         part3 = part3 .. string.format(
       "ugen_%s_init_data(%s_%i_%02i_data);\n",
       ugen["name"], ugen["name"], i - 1, ugen["id"])
      end  
   end
   local outstr = part1 .. "\n    unsafe {\n" .. part2 .. part3 .. "    }\n"
   return outstr
end

function gen_DSP_Server_Process(ugens)
   local outstr = ''
   local part1 = ''
   local part4 = ''
   for i, chain in ipairs(ugens) do
      part2 = ''
      part3 = ''
      for j, ugen in ipairs(chain["ugens"]) do
	 if i - 1 == i then
	    if part2 == '' then prefix = '    if' else prefix = 'else if' end
            part2 = part2 .. prefix .. string.format([[ (data_ready_bits[%i] & (1 << (%s_%i_%02i_CONTROLS & 0x3F))) {
                    ugen_id = %s_%i_%02i_CONTROLS;
		} ]], i,
		string.upper(ugen["name"]), i - 1, ugen["id"],
		ugen["name"], i - 1, ugen["id"])
	    if part3 == '' then prefix = '    if' else prefix = 'else if' end
            part3 = part3 .. prefix .. string.format([[ (data_ready_bits[%i] & (1 << (%s_%i_%02i_CONTROLS & 0x3F))) {
                    new_dsp_ptr = %s_%i_%02i_data;
                    %s_%i_%02i_data = dsp_pointer;
		} ]], i,
			     string.upper(ugen["name"]), i - 1, ugen["id"],
			     ugen["name"], i - 1, ugen["id"],
			     ugen["name"], i - 1, ugen["id"])

		     
	 end
	 if i - 1 == i then
	    part4 = part4 .. string.format([[
            case  %s_%i_%02i_CONTROLS:
                unsafe {
                    memcpy( %s_%i_%02i_data, data.data, sizeof(ugen_%s_data_t));
                }
                data_ready_bits[0] |= 1 << (%s_%i_%02i_CONTROLS & 0x3F);
                dsp_params[0].data_ready();
                break;
]], 
			     string.upper(ugen["name"]), i - 1, ugen["id"],
			     ugen["name"], i - 1, ugen["id"],
			     ugen["name"],
	 string.upper(ugen["name"]), i - 1, ugen["id"])
	 end
      end
      part1 = part1 
	 .. string.format("case dsp_params[%i].get_data_id() -> ugen_id_t ugen_id:\n",
			  i)
      part1 = part1 .. part2 .. '\n'
      part1 = part1 
	 .. string.format("case dsp_params[%i].get_data(void * unsafe dsp_pointer) -> void * unsafe new_dsp_ptr:\n            unsafe {\n",
			  i)
      part1 = part1 .. part3 .. '\n        }\n        break;\n'
      part1 = part1 .. [[        case server_xchange.data_ready():
            xchange_t data = server_xchange.get_data();
            switch (data.index) {
]]
   end

   outstr = '    while (1) {\n        select {\n' .. part1
   outstr = outstr .. part4 .. [[
            }
            break;
        }
   }
]]
    
   return outstr
end


function gen_Main_DSP_includes(ugens)
   outstr = ''
   for i=0, gen_calculate_threads(ugens)-1 do
      outstr = outstr .. '#include "dsp_' .. i .. '.h"\n'
   end
   return outstr
end

function gen_Main_DSP(ugens)
   local outstr = ''
   for i=0, gen_calculate_threads(ugens)-1 do
      outstr = outstr .. 
	 string.format('       on tile[DSP_TILE]: dsp_%i(c_aud_dsp[%i], dsp_params[%i], dsp_buffers_A[%i]);\n', i,i,i,i)
   end
   return outstr
end

function gen_Server_Controls(ugens)
   list = ''
   for i, chain in ipairs(ugens) do
      for j, ugen in ipairs(chain["ugens"]) do
      list = list .. string.format('     %s_%i_%02i_CONTROLS,\n',
				   string.upper(ugen["name"]), i - 1, ugen["id"])
      end
   end
   local outstr = 'typedef enum {\n' .. list .. '} ugen_id_t;\n'
   return outstr
end

function gen_Server_Buffers(ugens)
   list = ''
   for i, chain in ipairs(ugens) do
      for j, ugen in ipairs(chain["ugens"]) do
         ugen_data_size = 6;
         -- FIXME get data size from ugen database
         list = list .. string.format('     S32_T %s_%i_%02i_data[%i];\n',
                  ugen["name"], i - 1, ugen["id"],
                  ugen_data_size
               )
      end
   end
   local outstr = 'typedef struct {\n' .. list .. '} swap_buffers;\n'
   return outstr
end


