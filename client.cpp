// ----------------------------------------------------------------------------
// (CSS 432) Computer Networking (Spring 2016) - Reza Naeemi
// University of Washington Bothell
// Professor:  Robert Dimpsey
//
// Program 1 (Sockets) - client.cpp
// ----------------------------------------------------------------------------
// This assignment is intended for two purposes: (1) to exercise use of various
// socket-related system calls and (2) to evaluate dominant overheads of
// point-to-point communication over 1Gbps networks.  The program will use the
// client-server model where a client process establishes a connection to a
// server, sends data or requests, and closes the connection while the server
// sends back responses or acknowledgments to the client.
// ----------------------------------------------------------------------------
// usage: client [port] [repetition] [nbufs] [bufsize] [server] [type]
//
// Note:  The product of [nbufs] and [bufsize] must be (BUFSIZE) which is 1500.


#include <sys/types.h>		// socket, bind
#include <sys/socket.h>		// socket, bind, listen, inet_ntoa
#include <netinet/in.h>		// htonl, htons, inet_ntoa
#include <arpa/inet.h>		// inet_ntoa
#include <netdb.h>		// gethostbyname
#include <unistd.h>		// read, rite, close
#include <string.h>		// bzero
#include <netinet/tcp.h>	// SO_REUSEADDR
#include <sys/uio.h>		// writev

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <iostream>

const static int ARG_COUNT = 7;

const static int BUFSIZE = 1500;

const static int VALID_PORT_RANGE_BEGIN = 1024;
const static int VALID_PORT_RANGE_END = 65536;

const static int VALID_TYPE_NUMBER_BEGIN = 1;
const static int VALID_TYPE_NUMBER_END = 3;


// Displays usage information
void displayUsageInfo()
{
	std::cout << "usage: client [port] [repetition] [nbufs] [bufsize] [server] [type]" << std::endl;
        std::cout << std::endl;
        std::cout << "There are six arguments required for this command:" << std::endl;
        std::cout << "   port            Server IP port to bind" << std::endl;
        std::cout << "   repetition      The repetition of sending a set of data buffers" << std::endl;
        std::cout << "   nbufs           The number of data buffers" << std::endl;
        std::cout << "   bufsize         The size of each data buffer (in bytes)" << std::endl;
        std::cout << "   server          A server IP address or name" << std::endl;
        std::cout << "   type            The type of transfer scenario: {1, 2, or 3}" << std::endl;
        std::cout << std::endl;
        std::cout << "Transfer scenarios:" << std::endl;
        std::cout << "   Type 1:  Multiple writes resulting in calling as many writes as the number of" << std::endl;
        std::cout << "            data buffers (nbufs)." << std::endl;
        std::cout << "   Type 2:  Allocate an array of iovec data structures." << std::endl;
        std::cout << "   Type 3:  Single write allocating an nbufs-sized array of data buffers." << std::endl;
		std::cout << std::endl;
}


// Check if is allowed port number range
bool isInvalidPortNumber(int port)
{
	return ((port < VALID_PORT_RANGE_BEGIN) ||
		(port > VALID_PORT_RANGE_END));
}


// Check if repetition number is a negative number
bool isNegativeRepetitionNumber(int repetition)
{
	return (repetition < 0);
}


// Check to make sure the buffer size is correct
bool isIncorrectBufferSizeTotal(int nbufs, int bufsize)
{
	return ((nbufs * bufsize) != BUFSIZE);
}


// Check to make sure type number is valid
bool isInvalidTypeNumber(int type)
{
	return ((type < VALID_TYPE_NUMBER_BEGIN) ||
		(type > VALID_TYPE_NUMBER_END));
}



int main(int argc, char* argv[])
{	
	// Call displayUsageInfo() function, 
	// if the argument count is not equal to the ARG_COUNT.
	// Then return -1. 
	if (argc != ARG_COUNT)
	{
		displayUsageInfo();


		return -1;
	}

		
	// Use the isInvalidPortNumber(...) function
	// to check if the port number is in the valid range.
	if (isInvalidPortNumber(atoi(argv[1])))
	{
		std::cerr << "Error: " << 
		argv[1]  <<
		" is not a valid port number in the allowed range." <<
		" (" <<
		VALID_PORT_RANGE_BEGIN <<
		" - " <<
		VALID_PORT_RANGE_END <<
		")" << 
		std::endl;


		return -1;
	}

	
	// Use the isNegativeRepetitionNumber(...) function
	// to check if the repetition is negative.
	// Then return -1.
	if (isNegativeRepetitionNumber(atoi(argv[2])))
	{
		std::cerr << "Error: " <<
                argv[2]  <<
                " is not a positive repetition number." <<
		" (Valid repetition numbers are positive.)" <<
                std::endl;


		return -1;
	}	


        // Use the isIncorrectBufferSize(...) function
        // to check if the product of the number of buffers and
	// each buffer size is not equal to the BUFSIZE.
        // Then return -1.
        if (isIncorrectBufferSizeTotal(atoi(argv[3]), atoi(argv[4])))
	{
                std::cerr << "Error: The product of " <<
                argv[3] << 
		" and " <<
		argv[4] <<
                " does not result in the correct buffer size total." <<
                " (The product of nbufs and bufsize" << 
		" should be equal to BUFSIZE: " <<
		BUFSIZE <<
		std::endl;


                return -1;
        }



        // Use the isInvalidTypeNumber(...) function
        // to check if the type is invalid.
	// Then return -1.
        if (isInvalidTypeNumber(atoi(argv[6])))
        {
                std::cerr << "Error: " <<
                argv[6] <<
                " is an invalid type number." <<  
                " (1 - 3" <<
                std::endl;


                return -1;
        }



	int port = atoi(argv[1]);	// the last 5 digits of your student id (UW SID)
	int repetition = atoi(argv[2]);
	int nbufs = atoi(argv[3]);
	int bufsize = atoi(argv[4]);
	const char* serverIp = argv[5];
	int type = atoi(argv[6]);
	

	// Get Hostent structure to use for communication with the serverIp/
	struct hostent* host = gethostbyname(serverIp);
	if (!host)
	{
		std::cerr << "Error: " <<
		host << " was not found on the network." <<
		std::endl;

		
		return -1;
	}	


		
	// Construct the sending socket address for the client.
	sockaddr_in sendSockAddr;
	bzero((char*)&sendSockAddr, 
	sizeof(sendSockAddr));
	
	// Address Family: Internet
	sendSockAddr.sin_family = AF_INET;
	sendSockAddr.sin_addr.s_addr = 
		inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));

	sendSockAddr.sin_port = htons(port);


	// Open the TCP socket
	int clientSD = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSD < 0)
	{
		std::cerr << "Error: Failed to open socket." << 
		std::endl;

		close(clientSD);
		

		return -1;
	}

	
	// Connect the TCP socket with the Server
	int returnCode = 
		connect(clientSD, 
		(sockaddr* )&sendSockAddr, sizeof(sendSockAddr));

	if (returnCode < 0)	// Return codes should be positive.
	{			// otherwise, the connection failed.
		std::cerr << "Error: Failed connection" << std::endl;

		close(clientSD);
		

		return -1;
	}

	
	// Allocate the data buffer
	char databuf[nbufs][bufsize];

	
	// Use timeval structs for gettimeofday
	// to keep track of the total usage time.
	struct timeval start;
	struct timeval lap;
	struct timeval stop;
	long lapTime;
	long totalTime;

	// Log the start time.
	gettimeofday(&start, NULL);

	for (int i = 0;  i < repetition; i++)
	{
		// Invoke the write() system call for muliple writes
		// for each of the data buffers, so that there are as many writes
		// as there are number of buffers.
		
		switch (type)
		{
			// multiple writes:
			// Invoke the write() system call for multiple writes
			// for each of the data buffers, 
			// so that there are many writes
			case 1:
			{
				for (int j = 0; j < nbufs; j++)
				{
					write(clientSD, 
					databuf[j],
					bufsize); 
				}	
			
				break;
			}				

			// writev: 
			// Allocates an array of iovec data structures,
			// each having its *iov_base field point to another
			// another data buffer,
			// as well as storing the size of the buffer in iov_len.
			// Then calling writev() to send all the data buffers
			// together at one time.
			case 2:
			{				
				struct iovec vector[nbufs];
				for (int j = 0; j < nbufs; j++)
				{
					vector[j].iov_base = databuf[j];
					vector[j].iov_len = bufsize;
				}

				writev(clientSD, vector, nbufs);

				break;
			}
	
			// single write:
			// Allocates an array the size of nbufs.
			// Then calls write() to send the array of all data
			// buffers together at one time.	 
			case 3:
			{
				write(clientSD, 
				databuf, 
				nbufs * bufsize);

				break;
			}
		}
			
	}

	
	// Log write end time
	gettimeofday(&lap, NULL);


	// Get acknowledgement (an Integer) from the serverIp that shows how many
	// times the serverIp called the read() function. 
	int count;
	read(clientSD, &count, sizeof(count));

	
	// Log total rount-trip end-time.
	gettimeofday(&stop, NULL);


	// Output statistics:
	lapTime = 1000000 *
		(lap.tv_sec - start.tv_sec) +
		(lap.tv_usec - start.tv_usec); 

	totalTime = 1000000 *
		(stop.tv_sec - start.tv_sec) +
		(stop.tv_usec - start.tv_usec);

	
	std::cout << "Test " << type << ":";
	std::cout << "Data-sending time: " << lapTime << " usec";
	std::cout << "Round-Trip time: " << totalTime << " usec";
	std::cout << "Number of Reads: " << count << std::endl;  


	// End the session
	close(clientSD);	


	return 0;
}
