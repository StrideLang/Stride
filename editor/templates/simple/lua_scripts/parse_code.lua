
local re = require "re"

-- find the position of the first numeral in a string
print(re.find("the number 423 is odd", "[0-9]+"))  --> 12    14

-- returns all words in a string
print(re.match("the number 423 is odd", "({%a+} / .)*"))
--> the    number    is    odd

-- returns the first numeral in a string
print(re.match("the number 423 is odd", "s <- {%d+} / . s"))
--> 423

print(re.gsub("hello World", "[aeiou]", "."))
--> h.ll. W.rld

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
