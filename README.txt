TCP Server

TCServerDisplay.c
This program is a stream (TCP) server installed at a port # provided by the user
through the command line. This program displays a received segment one byte at a time in hexadecimal.

How to use it?

1) Compile TCP Server:
	$ g++ -o ServerTCP.out ServerTCP.c
2) Run TCP Server:
	$ ./ServerTCP.out <port number>
3) Compile TCP Client:
	$ javac ClientTCP.java
4) Run TCP Client:
	$ java ClientTCP <hostname> <port number> <operation> <message>



