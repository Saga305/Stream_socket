#####################################################
#		    Tetings                         #
#####################################################

Compilation:
	server:
		c++ server.cpp -o server -lpthread

	client:
		c++ client.cpp -o client

Run:
	./server

	./client 120 TEST a@1235 5

Note:
	=>Server will send a file called readMe.txt to client upon successful login.
	=>Client will create a file called RecievedFile_Uniqid(UniqID recived from server) in PWD.
