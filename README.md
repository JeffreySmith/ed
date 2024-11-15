This is a project to re-implement the posix editor [ed](https://pubs.opengroup.org/onlinepubs/009695299/utilities/ed.html) in c++ with some additional features like editline support. If running ed++ in OpenBSD, [pledge](https://man.openbsd.org/pledge.2) is enabled.

# Build instructions
```
git clone https://github.com/JeffreySmith/ed.git
cd ed
mkdir build
cd build
cmake .. && make
./ed++ [-p string] [-v] [filename]
```
