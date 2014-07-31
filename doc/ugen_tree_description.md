ugen_tree description
---------------------

ugen_tree is a lua table containing the independent signal processing chains. Each chain is to rendered on a separate parallel core whenever possible:

		ugen_tree = { chain1, chain2 }

If there is cross-talk between two channels, they will need to be rendered in the same "chain", so the ugen_tree would have only one element:

		ugen_tree = { channels1_2 }

Each chain is a table with one or more ugens table. Each "ugen" can have the following members:

* ugen: The name of the ugen
* token: A number that uniquely identifies each one of the outputs of the ugen
* input: If the ugen takes input signals, this table identifies the tokens that are used as its input

Builtin operations are prefixed in their ugen name by ":". These operations do not require a "token" member, unless it's needed (e.g. for :adcin that needs to pass the signal to another ugen). The available operations are:

* :dacout
* :adcin

Binary operations:
* :add
* :mult
* :subt
* :div

Some examples (for 2 channel input and output):

1. Noise going into a filter

		chain1 = { {ugen = "noise", token = {1}}, 
			{ugen = "biquad", token = {0}, input = {1} }
			{ugen = ":dacout", input = {0, 0}}
			}

2. Ring modulation

		chain1 = { {ugen = ":dacin", token = {0, 1}},
			{ugen = "sine", token = {2}},
			{ugen = ":mult", token = {3}, input = {0, 2}},
			{ugen = ":dacout", input = {3, 3}}
			}


TODO: How to model control information coming in.
TODO: How to model conditional paths?
TODO: how to model a miniDSP like system where there is parallelism then something than needs to be processed together, followed again by parallelism?

