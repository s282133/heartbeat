# heartbeat

Project developed using QEMUARM as target machine.

## brief description of the work
**heartbeat_app.c** uses 2048 threads in order to read data and one in order to compute data.
Each thread reads one value retrieving it from the module. I chose to have a thread for each read operation in order to simulate a real context in which a read operation may fail but the whole task (read + compute operations) doesn't.  
Threads got defined as detached in order to save memory.  
Timings are hard to evaluate on the virtual machine but I tried to get as close as I could to the ideal timings by decreasing the parameter stating the idle time in usleep function, obtaining an error of 0.35%.

## deployment of the app
Assuming that the machine is setup, the steps to be followed in order to deploy the app are: 
  1) copy the directory into _~/poky/_
  2) launch _source oe-init-build-env build_qemuarm/_ command
  3) in _/poky/build_qemuarm/conf/local.conf_ add the following lines:  
  IMAGE_INSTALL_append = " mymodextended"
  KERNEL_MODULE_AUTOLOAD += "mymodextended"   
  IMAGE_INSTALL_append = " heartbeat" 
  4) bblayer
  4) launch _bitbake core-image-minimal_ command
  5) start qemuarm machine
  6) retrieve the app going to _/usr/bin_
  7) launch the app with _./heartbeat_app_
