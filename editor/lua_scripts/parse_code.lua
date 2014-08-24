
function parse(code)
   print("Start parsing.")
   print(code)

   local chain1 = { output = {0},
      ugens = { {name = "noise", id = 0},
         { name = "biquad", id = 1, input = {0} } }
      }
         
   local chain2 = { output = {1}, ugens = {
         {name = "noise", id = 2},
         {name = "biquad", id = 3},
         {name = ":add", id= 4, input = {2,3} }
         }
      }

   local ugens = { chain1, chain2 }   
   return ugens
end

local code = "# hello\n Noise >> Biquad\n"

parse(code)