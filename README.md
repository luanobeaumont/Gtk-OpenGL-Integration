# OpenGL-Gtk Integration Application

Integration of Gtk as wrapper for OpenGL.

##
Gtk3.0
OpenGL

## Build Instructions
*   Run this bash command to compile the code.
```bash
g++ main.cpp -o <appnameexecutable> $(pkg-config --cflags --libs gtk+-3.0 epoxy)
```

*   Then run the executable file
```bash
./<appnameexecutable>
```
