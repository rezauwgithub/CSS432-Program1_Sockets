// ----------------------------------------------------------------------------
// CSS 432 (Spring 2016) - Reza Naeemi
// University of Washington Bothell
// Professor:  Robert Dimpsey
//
// Program 1 (Sockets) - server.cpp
// ----------------------------------------------------------------------------
// This assignment is intended for two purposes: (1) to exercise use of various
// socket-related system calls and (2) to evaluate dominant overheads of
// point-to-point communication over 1Gbps networks.  The program will use the
// client-server model where a client process establishes a connection to a
// server, sends data or requests, and closes the connection while the server
// sends back responses or acknowledgments to the client.
// ----------------------------------------------------------------------------
// Usage: server [port] [repetition]
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

#include <signal.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include <fcntl.h>

#include <iostream>


const static int ARG_COUNT = 3;

const static int BUFSIZE = 1500;

const static int VALID_PORT_RANGE_BEGIN = 1024;
const static int VALID_PORT_RANGE_END = 65536;

const static int NUMBER_OF_CONNECTIONS = 10;


// Displays usage information
void displayUsageInfo()
{
        std::cout << "usage: server [port] [repetition]" << std::endl;
        std::cout << std::endl;
        std::cout << "There are two arguments required for this command:" << std::endl;
        std::cout << "   port           Server IP port to bind" << std::endl;
        std::cout << "   repetition     The repetition of sending a set of data buffers" << std::endl;
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



int repetition;		// Server Repetitions
int serverSD;		// Socket Descriptor
int newSD;		// File Descriptor


void handler(int sig)
{
	// Allocate the data buffer.
	char dataBuf[BUFSIZE];


	// Use timeval structs for gettimeofday
	// to keep track of the total usage time.
	struct timeval start;
	struct timeval stop;
	long totalTime;

	// Log the start time.
	gettimeofday(&start, NULL);

	
	// Call read() for each repetition.
	// Documentation notes that the read() system call
	// may not return all the data if network performance is degraded (slow). 	
	int count = 0;
	int nRead;
	for (int i = 0; i < repetition; i++)
	{
		nRead = 0;
		while (nRead < BUFSIZE)
		{
			int bytesRead = read(newSD, dataBuf, BUFSIZE - nRead);
			nRead += bytesRead;
			count++;
		}
	}

	
	// Log the end time.
	gettimeofday(&stop, NULL);

	
	// Calculate the total time used and report it.
	totalTime = 1000000 * (stop.tv_sec - start.tv_sec) +
			(stop.tv_usec - start.tv_usec);

	std::cout << "Time data as received: " << 
	totalTime << " usec" << 
	std::endl;


	// Send the count for number of read() calls,
	// as an acknowledgement to the the client.
	write(newSD, &count, sizeof(count));


	// Terminate the server process
	close(newSD);
	close(serverSD);

	exit(0);

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
	

	int port = atoi(argv[1]);
	repetition = atoi(argv[2]);

	
	// Construct the receiving socket address for available server interface.
	sockaddr_in acceptSockAddr;
	bzero((char*)&acceptSockAddr, sizeof(acceptSockAddr));
	acceptSockAddr.sin_family = AF_INET;	// Address Family: Internet
	acceptSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	acceptSockAddr.sin_port = htons(port);


	// Open the TCP socket.
	serverSD = socket(AF_INET, SOCK_STREAM, 0);
	const int ON = 1;
	setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char*)&ON, sizeof(int));


	// Bind the TCP socket.
	int returnCode = 
		bind(serverSD, (sockaddr*)&acceptSockAddr, sizeof(acceptSockAddr));

	if (returnCode < 0)	// Return codes should be positive.
	{			// otherwise, the connection failed.	
		std::cerr << "Error: Failed binding." << std::endl;
		
		close(serverSD);

		
		return -1;	
	}	
	
		
	// Listen for a connetion
	listen(serverSD, NUMBER_OF_CONNECTIONS);
	sockaddr_in newSockAddr;
	socklen_t newSockAddrLength = sizeof(newSockAddr);


	// Wait for a client to connect
	newSD = accept(serverSD, (sockaddr*)&newSockAddr, &newSockAddrLength);


	// Set up a signal handle for SIGIO signals.
	// SIGIO signals will cpome from socket IO calls from the client.
	signal(SIGIO, handler);


	// Change the socket into an asynchronous connection
	// using fcntl commands as follows.
	fcntl(newSD, F_SETOWN, getpid());
	fcntl(newSD, F_SETFL, FASYNC);	 
	
	
	// The handler ill terminate the server process using interrupt.
	// Until that happens, the server will not end on its on.
	// Instead, it ill sleep for 1 second. (1000ms = 1 sec)
	while (true)
	{
		sleep(1000);
	}


	
	return 0;
}
	
