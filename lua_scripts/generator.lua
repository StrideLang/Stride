print("Hello from Lua")

local ugens = {{name = "biquad", tile = 0, dspcore = 0, id = 0},
{name = "biquad", tile = 0, dspcore = 0, id = 1},
{name = "biquad", tile = 0, dspcore = 1, id = 0}
}

function get_ugens()
  return ugens
end

function gen_Compute_Process(ugens)
  outstr = ''
  for i, ugen1 in ipairs(ugens) do
  outstr = outstr .. string.format([[
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
]], ugen1["name"], ugen1["name"], ugen1["dspcore"],
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
  outstr = outstr .. [[            memcpy(data.data, server_ctr_data.data_buffer, datasize);
  xchange_data = data;

  control_pending = -1;
  break;

  case comps.get_compute_data() -> control_data_t * unsafe ctr_data:
      server_ctr_data.index = control_pending;
      unsafe {
          switch(control_pending) {
          ]] .. part2 .. [[                    }
          }
      break;
      ]]
  return outstr
end

function myfunction(arg)
  return cppfunction(arg)
end

print(gen_Control_Server_Control_Process(ugens))
