Build instructions



Debian system (EASY)


1. Assumes that you have a kernel that is gunzipped / untarred  into /usr/src

2. Assumes that uname -r matches the kernel you are currently using and have installed

3. If these are met, then you should just be able to run "make"




Your Kernel source exists elsewhere (LITTLE TRICKY)


1. Specify the path to your kernel code, as follows


make KDIR=/usr/src/linux-3.4.37 







