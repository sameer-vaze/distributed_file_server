Distributed File Server

This project submission contains two .c files that can be compiled to run a TCP distributed fileserver.

---dfs.c---
Once compiled an executable server is created. The arguements passed are:
	
	1. DFS directory
	2. Port number

Execution is done by the following way.
Example: ./dfs <directory of server> <port number>

Compiled using: gcc dfs.c -o dfs

Based on the arguements passed we can create as many number of distributed servers as we want.

---dfc.c---
Once compiled an executable client is created. The arguements passed are:

	1. Configuration file

Example: ./dfc <configuration file name>

Compiled using: gcc dfc.c -o dfc -lcrypto -lssl

We can have multiple clients that connect to the individual servers as required.

Functions Implemented:

	1. list
		This function lists all the files that are present at the server side

	2. get
		This function retrieves the file as required from the servers and recombines
		the pieces to form the complete file

	3. put
		This function breaks the files into multiple pieces and then stores these 
		pieces in the multiple file servers

	4. exit
		This function exits all the client and server instances gracefully

Special features:

	1. Subfolder access
		By specifying the subdirectory, the program can store, retrieve and list all 
		files in the subdirectory mentioned

	2. Efficient retrieval/Traffic optimization
		This selects the servers to retrieve from in such a way that one piece is 
		retrieved only one time




