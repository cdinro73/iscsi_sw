Setup for test scenario

Step 1: 
Create virtual machines running CentOs. Use the following IP addresses:
Initiator: 192.168.83.128
Switch1: 192.168.83.130
Switch2: 192.168.83.131
Target: 192.168..83.129
	
Step 2: 
Dearchivate the archive iscsi_sw.tgz 
The directory structure is:
-	include
-	src
-	utils
-	doc
For compiling, navigate in src directory and run make.
For installing the database, run mysql <alldb.sql
Edit the table lookup and setup the paths. You can use phpMyEdit for table editing.
Example of table content:

SW1:
 

SW2:
 

Step 3:
On each switch, launch the log server from the src directory with the command ./logsrv &
On each switch launch the main application using the command: ./router –d 127.0.0.1
Make sure the iscsi target daemon is running on the target machine.

Step 4:
On the initiator, run the commands:
iscsiadm -m discovery -t sendtargets -p 192.168.83.131
/etc/init.d/iscsi start
fdisk –l to see the new iscsi disk
mount the new iscsi disk



Step 5:
In the utils directory, there is the TracerClient.java console. Compile it with javac TracerClient.java and launch the console with the command java TracerClient.
Connect to each switch on port 1973 to see log and debug messages.

