Continuous Perfomance Monitor (CPM)
###################################

Simple tool to monitor the performance of a program in the long run. 

Build
+++++

The benchmark helper is header-only, but the report generator needs to be built. You simple need to use make for building and installing it: 

.. code:: bash

   make
   sudo make install

You need a recent compiler to build cpm. It has been tested with clang++-3.4 and greater and g++-4.9 and greater. 
