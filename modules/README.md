# Loadable modules

Here goes all modules which end up loaded in runtime.
These modules are compiled as position independent code and linked against kernellib.o, 
which is is a library containing the absolute positions of all kernel functions in memory.
Every time kernel is recompiled these modules need to be recompiled as well.
