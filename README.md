FinalMQ - Message exchange framework
===================================================


Copyright 2020 bexoft GmbH

C++ Installation - Unix
-----------------------

To build finalmq from source, the following tools and dependencies are needed:

  * git
  * make
  * cmake
  * g++
  * openssl
  * libfcgi
  * node-ejs
  * node-minimist

On Ubuntu/Debian, you can install them with:

    $ sudo apt-get install -y git make cmake g++ libssl1.0-dev libfcgi-dev node-ejs node-minimist

To get the source, clone the repository of finalmq:

    $ git clone https://github.com/bexoft/finalmq.git
    
To build and install the C++ finalmq execute the following:

     cd finalmq
     mkdir build
     cd build
     cmake ..
     make
     sudo make install
     sudo ldconfig # refresh shared library cache.


**Hint on install location**

By default, the package will be installed to /usr/local.  However,
on many platforms, /usr/local/lib is not part of LD_LIBRARY_PATH.
You can add it, but it may be easier to just install to /usr
instead.  To do this, invoke cmake as follows:

    cmake -DCMAKE_INSTALL_PREFIX=/usr ..

If you already built the package with a different prefix, make sure
to run "make clean" before building again.


C++ Installation - Windows
--------------------------

To build from source using VC++, follow this instructions:
* Install dependend tools and dependencies listed above (TODO: details will follow)
* Open CMakeLists.txt from VisualStudio
* Compile the finalmq project

To build from source using Cygwin or MinGW, follow the Unix installation
instructions, above.


