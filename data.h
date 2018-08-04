
#include <cstdlib>


/* Requst Structure */
typedef struct loginRequest_s
{
	unsigned short int transCode;
	char loginId[11];
	char password[11];
	unsigned short int version;

	loginRequest_s ():transCode(0),version(0)
	{
		(void) memset(loginId, '\0', sizeof(loginId));
		(void) memset(password, '\0', sizeof(password));
	}
}loginReq;

/* Response structure */
typedef struct loginResponse_s
{
	unsigned short int unqId;
	int logOnTime;
	char rejectReason[51];

	loginResponse_s ():unqId(0),logOnTime(0)
	{
		(void) memset(rejectReason, '\0', sizeof(rejectReason));  
	}

}loginRes;
