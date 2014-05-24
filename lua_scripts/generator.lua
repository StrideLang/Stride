
local num_dsp_threads = 2

function get_ugens()
   return ugens
end

function gen_Compute_Process(ugens)
   outstr = ''
   for i, ugen1 in ipairs(ugens) do
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
  string.upper(ugen1["name"]),ugen1["dspcore"], ugen1["id"],
  ugen1["name"] ,ugen1["name"],ugen1["name"],
  ugen1["dspcore"], ugen1["id"], "cf",
  ugen1["name"], ugen1["name"], ugen1["dspcore"], ugen1["id"])
   end
   return outstr
end

function gen_Control_Server_Ugen_Controls(ugens)
   outstr = ''
   for i, ugen1 in ipairs(ugens) do
      outstr = outstr .. string.format([[
        ugen_%s_controls_t %s_%i_%02i_controls;
]],
        ugen1["name"], ugen1["name"], ugen1["dspcore"], ugen1["id"])
   end
   return outstr
end

function gen_Control_Server_Ugen_Init(ugens)
   part1 = ''
   part2 = ''
   for i, ugen1 in ipairs(ugens) do
      part1 = part1 .. string.format([[
      ugen_%s_init_controls(server_ctr_data.%s_%i_%02i_controls);
]], ugen1["name"], ugen1["name"], ugen1["dspcore"], ugen1["id"])
  part2 = part2 .. string.format([[
    ugen_%s_init_data((ugen_%s_data_t *) buffers[%i].%s_%i_%02i_data);
   ]],
      ugen1["name"], ugen1["name"], ugen1["dspcore"],
      ugen1["name"], ugen1["dspcore"], ugen1["id"])
   end
   outstr = part1 .. "unsafe {\n" .. part2 .. "\n}\n"
   return outstr
end

function gen_Control_Server_Control_Process(ugens)
   part1 = ''
   part2 = ''
   for i, ugen1 in ipairs(ugens) do
      part1 = part1 .. string.format([[
case %s_%i_%02i_CONTROLS:
  datasize = sizeof(ugen_%s_data_t);
  break;
]], string.upper(ugen1["name"]), ugen1["dspcore"], ugen1["id"], ugen1["name"])
      part2 = part2 .. string.format([[
  case %s_%i_%02i_CONTROLS:
          server_ctr_data.data_buffer =  buffers[%i].%s_%i_%02i_data;
	  ctr_data = &server_ctr_data;
	  server_ctr_data.value = button_val == BUTTON_DOWN ? 1:0;
	  break;
   ]], string.upper(ugen1["name"]), ugen1["dspcore"], ugen1["id"], ugen1["dspcore"],
   ugen1["name"], ugen1["dspcore"], ugen1["id"])
   end
   outstr = "switch(control_pending) {\n" .. part1 .. "}\n"
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
   outstr = ''
   for i, ugen1 in ipairs(ugens) do
      if ugen1["dspcore"] == dsp_index then
	 outstr = outstr .. string.format([[
      ugen_%s_state_t %s_%i_%02i;
]],
      ugen1["name"], ugen1["name"], ugen1["dspcore"], ugen1["id"])
      end
   end
   return outstr
end


function gen_DSP_Init_Ugens(ugens, dsp_index)
   outstr = ''
   for i, ugen1 in ipairs(ugens) do
      if ugen1["dspcore"] == dsp_index then
	 outstr = outstr .. string.format([[
        ugen_%s_init(%s_%i_%02i, (ugen_%s_data_t *) &(buffer.%s_%i_%02i_data[0]));
   ]],
      ugen1["name"], ugen1["name"], ugen1["dspcore"], ugen1["id"],
      ugen1["name"], ugen1["name"], ugen1["dspcore"], ugen1["id"])
      end
   end
   return outstr
end

function gen_DSP_Process_Ugens(ugens, dsp_index)
   outstr = ''
   for i, ugen1 in ipairs(ugens) do
      -- This is the tricky part!
   end
   return outstr
end

function gen_DSP_Pointer_Swap(ugens, dsp_index)
   outstr = ''
   part = ''
   for i, ugen1 in ipairs(ugens) do
      if ugen1["dspcore"] == dsp_index then
	 part = part .. string.format([[
                    case (%s_%i_%02i_CONTROLS:
                        (%s_%i_%02i.data = dsp_params.get_data((%s_%i_%02i.data);
                        break;
		  ]],
		  string.upper(ugen1["name"]), ugen1["dspcore"], ugen1["id"],
		  ugen1["name"], ugen1["dspcore"], ugen1["id"],
		  ugen1["name"], ugen1["dspcore"], ugen1["id"])
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
   part1 = ''
   part2 = ''
   part3 = ''
   for i, ugen1 in ipairs(ugens) do
      part1 = part1 .. string.format(
	 "ugen_%s_data_t * unsafe %s_%i_%02i_data;\n",
	 ugen1["name"], ugen1["name"], ugen1["dspcore"], ugen1["id"])
      part2 = part2 .. string.format(
        "%s_%i_%02i_data = (ugen_%s_data_t *) buffers[%i].%s_%i_%02i_data;\n",
	 ugen1["name"], ugen1["dspcore"], ugen1["id"],
	 ugen1["name"], ugen1["dspcore"],
	 ugen1["name"], ugen1["dspcore"], ugen1["id"]
      )
      part3 = part3 .. string.format(
	 "ugen_%s_init_data(%s_%i_%02i_data);\n",
	 ugen1["name"], ugen1["name"], ugen1["dspcore"], ugen1["id"])
   end
   outstr = part1 .. "\n    unsafe {\n" .. part2 .. part3 .. "    }\n"
   return outstr
end

function gen_DSP_Server_Process(ugens)
   outstr = ''
   part1 = ''
   part4 = ''
   for i=0,num_dsp_threads-1 do
      part2 = ''
      part3 = ''
      for j, ugen1 in ipairs(ugens) do
	 if ugen1["dspcore"] == i then
	    if part2 == '' then prefix = '    if' else prefix = 'else if' end
            part2 = part2 .. prefix .. string.format([[ (data_ready_bits[%i] & (1 << (%s_%i_%02i_CONTROLS & 0x3F))) {
                    ugen_id = %s_%i_%02i_CONTROLS;
		} ]], i,
		string.upper(ugen1["name"]), ugen1["dspcore"], ugen1["id"],
		ugen1["name"], ugen1["dspcore"], ugen1["id"])
	    if part3 == '' then prefix = '    if' else prefix = 'else if' end
            part3 = part3 .. prefix .. string.format([[ (data_ready_bits[%i] & (1 << (%s_%i_%02i_CONTROLS & 0x3F))) {
                    new_dsp_ptr = %s_%i_%02i_data;
                    %s_%i_%02i_data = dsp_pointer;
		} ]], i,
			     string.upper(ugen1["name"]), ugen1["dspcore"], ugen1["id"],
			     ugen1["name"], ugen1["dspcore"], ugen1["id"],
			     ugen1["name"], ugen1["dspcore"], ugen1["id"])

		     
	 end
	 if ugen1["dspcore"] == i then
	    part4 = part4 .. string.format([[
            case  %s_%i_%02i_CONTROLS:
                unsafe {
                    memcpy( %s_%i_%02i_data, data.data, sizeof(ugen_%s_data_t));
                }
                data_ready_bits[0] |= 1 << (%s_%i_%02i_CONTROLS & 0x3F);
                dsp_params[0].data_ready();
                break;
]], 
			     string.upper(ugen1["name"]), ugen1["dspcore"], ugen1["id"],
			     ugen1["name"], ugen1["dspcore"], ugen1["id"],
			     ugen1["name"],
	 string.upper(ugen1["name"]), ugen1["dspcore"], ugen1["id"])
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


function gen_Main_DSP_includes()
   outstr = ''
   for i=0, num_dsp_threads-1 do
      outstr = outstr .. '#include "dsp_' .. i .. '.h"\n'
   end
   return outstr
end

function gen_Main_DSP()
   outstr = ''
   for i=0, num_dsp_threads-1 do
      outstr = outstr .. 
	 string.format('       on tile[DSP_TILE]: dsp_%i(c_aud_dsp[%i], dsp_params[%i], dsp_buffers_A[%i]);\n', i,i,i,i)
   end
   return outstr
end

function gen_Server_Controls(ugens)
   list = ''
   for i, ugen1 in ipairs(ugens) do
      list = list .. string.format('     %s_%i_%02i_CONTROLS,\n',
				   string.upper(ugen1["name"]), ugen1["dspcore"], ugen1["id"])

   end
   outstr = 'typedef enum {\n' .. list .. '} ugen_id_t;\n'
   return outstr
end

function gen_Server_Buffers(ugens)
   list = ''
   for i, ugen1 in ipairs(ugens) do
      list = list .. string.format('     S32_T %s_%i_%02i_data[%i];\n',
				   ugen1["name"], ugen1["dspcore"], ugen1["id"],
				   ugen1["ugen_data"]["size"]
				)

   end
   outstr = 'typedef struct {\n' .. list .. '} swap_buffers;\n'
   return outstr
end


