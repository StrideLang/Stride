
function parse(code)

   local ugens = {{name = "biquad", tile = 0, dspcore = 0, id = 0, ugen_data = {size = 6}},
                  {name = "biquad", tile = 0, dspcore = 0, id = 1, ugen_data = {size = 6}},
                  {name = "biquad", tile = 0, dspcore = 1, id = 0, ugen_data = {size = 6}}
                  }   
   return ugens
end
