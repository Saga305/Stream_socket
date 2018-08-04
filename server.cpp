#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>
#include <sys/time.h>
#include <cerrno>
#include <pthread.h>
#include "data.h"

using namespace std;

#define PORT 12345

struct timeval tp;
int millisec = 0;
static int unqId = 0;

void *processRequest(void *sock)
{
	int new_socket= *((int *) sock);
	loginReq req;
	loginRes res;
	char loginFailed[51] = "Login Fail invalid LoginId and Password";
	char transCodeNotMatched[51] = "Transcode is not matched";
	bool loggedIn = false;
	unsigned char buff[256];
	int buffSize = sizeof(buff);

	int valread = read( new_socket , &req, sizeof(req));
	cout <<"Recived request from client:\ntranscode:\t"<<req.transCode<<"\nloginId:\t"<<req.loginId<<"\npassword:\t"<<req.password<<"\nversion:\t"<<req.version<<endl;

	if(req.transCode == 120)
	{
		//login verification
		if(strcmp(req.loginId,"TEST") || strcmp(req.password,"a@12345"))
		{
			res.logOnTime = millisec;      
			memcpy(res.rejectReason, loginFailed,sizeof(loginFailed));
		}
		else
		{
			//fill up login response structure
			loggedIn = true;
			res.unqId = ++unqId;
			gettimeofday(&tp, NULL);
			millisec = tp.tv_sec * 1000 + tp.tv_usec / 1000;
			res.logOnTime = millisec;      
		}
	}
	else
	{
		memcpy(res.rejectReason, transCodeNotMatched,sizeof(transCodeNotMatched));
	}

	//sending a responce to the client
	send(new_socket , &res , sizeof(res) , 0 );
	cout<<"Response sent to client.."<<endl;

	if(loggedIn)
	{
		// Reading a file to send a data over a stream
		FILE *fp;
		fp = fopen("./ReadMe.txt", "r"); 

		if(fp == NULL)
		{
			perror("fopen:");
			return fp;
		}

		while(true)
		{
			//Read file in chunks of 256 bytes
			memset(buff,'\0',buffSize);
			int nread = fread(buff, sizeof(char), buffSize, fp);
			cout<<"Bytes read:"<<nread<<endl;        

			/* If read was success, send data. */
			if(nread > 0)
			{
				cout<<"Sending file's content"<<endl;
				write(new_socket, buff, nread);
			}

			if (nread < buffSize)
			{
				if (feof(fp))
				{
					cout<<"End of file"<<endl;
				}
				if (ferror(fp))
				{
					cout<<"Error reading"<<endl;
				}
				fclose(fp);
				break;
			}
		}
	}

}

int main(int argc, char const *argv[])
{
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};


	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket:");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to port 12345
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				&opt, sizeof(opt)))
	{
		perror("setsockopt:");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons( PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address, 
				sizeof(address))<0)
	{
		perror("bind:");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		perror("listen:");
		exit(EXIT_FAILURE);
	}

	while(1)
	{
		//accept blocks till the connect from the client
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
						(socklen_t*)&addrlen))<0)
		{
			perror("accept:");
			exit(EXIT_FAILURE);
		}

		//Creating a thread and sending socket file descriptor for communication
		int  status;
		pthread_t th;

		status=pthread_create(&th,
				NULL,
				processRequest,
				&new_socket
				);

		if(status!=0)
		{
			if(status==EPERM)
			{
				cerr<<"pthread_create got EPERM"<<endl;
			}
			else if(status==EINVAL)
			{
				cerr<<"pthread_create got EINVAL"<<endl;
			}
			else
			{
				cerr<<"pthread_create got neither EPERM nor EINVAL"<<endl;
			}

			cerr<<"pthread_create() got error:"<<status<<endl;
			errno=status;
			perror("pthread_create:");
			exit(1);
		}
		else
		{
			//When a detached thread terminates, its resources are automatically released back to the system without the need for another thread to join with the terminated thread.
			if (!pthread_detach(th))
			{
				cout<<"Thread detached successfully"<<endl;
			}
			else
			{
				perror("pthread_detach:");
			}
		}
	}
	return 0;
}
