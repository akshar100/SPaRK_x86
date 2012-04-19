SPaRK_x86
=========

Safety Critical Partitioned Kernel is a hypervisor based approach to run safety critical and non safety critical kernels on the same hardware platform. To know more visit: http://www.cse.iitb.ac.in/~akshar/MTP

Introduction
------------

Spark itself is a modified linux kernel. It is capable of running other operating systems on top of it. To enable a particular operating system to run on Spark we need to modify the Hardware Abstraction Layer (HAL) of that operating system such that instead of talking directly to hardware it talks with the SPARK kernel. Such operating Systems are called GuestOS.

30000 feet view
---------------
We first port a guestOs to Spark. This involved modifying the HAL of the GuestOS. Then we compile the GuestOS to generate the boot image. After generating all GuestOS images (there can be more than one GuestOS) we place these images in the Spark Folder. **./Spark/guestOSes**
