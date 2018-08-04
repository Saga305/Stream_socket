#include <cstdio>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include "data.h"

using namespace std;

#define PORT 12345

int main(int argc, char const *argv[])
{

	if(argc != 5)
	{
		cout << "Usage: Application_Name TransCode UserId Password Version " << endl;
		exit (EXIT_FAILURE);
	}

	loginReq req;
	loginRes res;
	unsigned char recvBuff[256];
	int recBufSize;
	FILE *fp;
	char fileName[256];
	struct sockaddr_in address;
	int sock, valread;
	struct sockaddr_in serv_addr;

	memset(&serv_addr, '\0', sizeof(serv_addr));
	memset(recvBuff,'\0',sizeof(recvBuff));
	memset(fileName,'\0',sizeof(fileName));
	sock = 0;
	valread = 0;
	recBufSize = sizeof(recvBuff);

	/* data infering*/
	req.transCode = atoi(argv[1]);
	(void) memcpy(req.loginId, argv[2], sizeof(req.loginId));    
	(void) memcpy(req.password, argv[3], sizeof(req.password));
	req.version = atoi(argv[4]);

	cout <<"Sending request to server:\ntranscode:\t"<<req.transCode<<"\nloginId:\t"<<req.loginId<<"\npassword:\t"<<req.password<<"\nversion:\t"<<req.version<<endl;


	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket:");
		exit (EXIT_FAILURE);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IP to binary form
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{
		perror("inet_pton:");
		exit (EXIT_FAILURE);
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Connect:");
		exit (EXIT_FAILURE);
	}

	//sending request
	send(sock , &req, sizeof(req) , 0 );
	cout<<"Request sent"<<endl;

	//wait for the responce from server
	valread = read( sock , &res, sizeof(res));
	cout <<"Got responce from server:\nunqId:\t"<<res.unqId<<"\nlogOnTime:\t"<<res.logOnTime<<"\nrejectReason:\t"<<res.rejectReason<<endl;

	if(res.rejectReason[0] != '\0')
	{
		cout<<"Login rejected:"<<res.rejectReason<<endl;
		exit (EXIT_FAILURE);
	}

	//construct file name according to uniq id received from server
	sprintf(fileName, "RecievedFile_%d", res.unqId);
	fp = fopen(fileName, "w"); 

	if(fp == NULL)
	{
		perror("fopen:");
		exit (EXIT_FAILURE);
	}

	while(true)
	{
		//waiting for file data from server
		valread = read(sock , recvBuff, sizeof(recvBuff));
		cout<<"valread: "<<valread<<endl;

		if(valread)
		{
			fwrite(recvBuff, sizeof(char), valread, fp);
		}

		if (valread < sizeof(recvBuff))
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
	return 0;
}
