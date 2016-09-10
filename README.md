# CTTP

A super lightweight and easy to use library for building a REST API 

-------

## Why use CTTP?
If you're going to use a shitty web framework like express (nodeJS), or PHP, or some sketchy ruby gem, why not use cttp and at least go really fucking fast.

## This sounds fantastic. Teach me senpai!
cttp uses the [makedeps](https://github.com/tmathmeyer/makedeps) makefile system to manage itself and its dependancy on the cref library. To build the example library, check out this repo, cd into the test directory, and run ````make binary````. Then you can run ````./build/binary/http [port_no]```` to prop up a server.

## Where can I read documentation?
The system at this point is so small and simple that an example of all functionality can be found in test/src/C/http.c. Eventually There will be real documentation once it can't all fit in there.

## Are there test cases?
Not right now, but there is a fancy debug mode. Enable it by putting -DDEBUG on the CFLAGS line in BOTH ```config.mk``` and ```test/config.mk```. Having one or the other, but not both, will certainly lead to segfaulting. If you'd like to submit a bug report, please do so with the debug mode turned on.

## Is this secure?
No. I only do security at work, I cant be bothered to do it here. I'm a little burned out of that stuff, ya know?

## I hate the [insert here] Feature!
too bad?
