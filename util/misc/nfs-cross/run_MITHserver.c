#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#define MAXPENDING 1		/* Max connection requests */
#define BUFFSIZE 400

void get_runlog (char * runlog)
{
    char *fp, *ep;
    char temp[100];
    int num2copy ;
    
    fp = strchr( runlog, '/' );
	fp++;
	
   	ep = strchr( fp, '.' );
   	num2copy= ep-fp;
   
	strncpy (temp, fp ,num2copy);
	sprintf(runlog, "../logs/%s.run1.log",  temp );
    fprintf(stdout,"The runlog is %s  \n",runlog);
    fflush(stdout);
   
    
}

void run_test( char * cmd)
{

    char *current_dir,runlog[100], temp[300];
    int pid, status,fd;		/* process identifier and status */
    fpos_t pos;
    
    current_dir= (char *)get_current_dir_name();
    if (chdir((const char *) "builds/linux/bin") != 0)
	perror("\nFailed to change dir");
    exit(1);
    
    /* construct runlog name from path recieved */
    strcpy (runlog,cmd);
    get_runlog(runlog);
 
    
    pid = fork();
    if (pid < 0) {
	perror("\nFailed to fork:");
	exit(1);
    }
    if (pid == 0)
    {
        
        /*Redirect stdout to test_name.run1.log*/
        
        sprintf(temp, "%s > %s", cmd, runlog );
        strcpy (cmd,temp);        
        system(cmd);        
        
        if (errno!=0)
        perror("\n child failed");
        exit(1);
        
        
        
        
    } else {
	fprintf(stdout,"Running test, it will take some time\n");
        while (wait(&status) != pid)
	    /* empty */ ;
    }
    
    chdir(current_dir);
    
}

void HandleClient(int sock)
{
    char exec[BUFFSIZE];
    char cmdline[BUFFSIZE];
    char *send_buffer = "DONE";
    int received = -1, bytes_to_send = 4;
	/* Receive message */
    if ((received = recv(sock, cmdline, BUFFSIZE, 0)) < 0) {
	perror("Failed to receive initial bytes from client");
    exit(1);
    }
    fprintf(stdout,"The cmdline is %s  \n",cmdline);
    fflush(stdout);
  
	/*do all the work here */
	run_test(cmdline);

    /* Send  Ack that  all the work is done */
    if (send(sock, send_buffer, bytes_to_send, 0) != bytes_to_send) {
	perror("Failed to send bytes to client");
    exit(1);
    }
    close(sock);
}
int main(int argc, char *argv[])
{
    int serversock, clientsock;
    struct sockaddr_in echoserver, echoclient;
    int opt;

    if (argc != 2) {
	fprintf(stderr, "USAGE:\n run_MITHserver <port_num>\n");
	exit(1);
    }
    /* Create the TCP socket */
    if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
	perror("Failed to create socket");
    exit(1);
    }
    opt = 1;
    if ( setsockopt(serversock,SOL_SOCKET,SO_REUSEADDR,(const void *)&opt, sizeof(opt)) == -1 ) 
    perror("setsockopt");
    exit(1);
    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));	/* Clear struct */
    echoserver.sin_family = AF_INET;	/* Internet/IP */
    echoserver.sin_addr.s_addr = htonl(INADDR_ANY);	/* Incoming addr */
    echoserver.sin_port = htons(atoi(argv[1]));	/* server port */

    /* Bind the server socket */
    if (bind(serversock, (struct sockaddr *) &echoserver,
	     sizeof(echoserver)) < 0) {
	perror("Failed to bind on the server socket");
	exit(1);
    }
    /* Listen on the server socket */
    if (listen(serversock, MAXPENDING) < 0) {
	perror("Failed to listen on socket");
    exit(1);
    }
    /* Run until cancelled */
    while (1) {
	unsigned int clientlen = sizeof(echoclient);
	/* Wait for client connection */
	fprintf(stdout,"Waiting for client connection\n");
	if ((clientsock =
	     accept(serversock, (struct sockaddr *) &echoclient,
		    &clientlen)) < 0) {
	    perror("Failed to accept client connection");
	    exit(1);
	}
	fprintf(stdout, "Client connected: %s\n",
		inet_ntoa(echoclient.sin_addr));
	HandleClient(clientsock);
    }
}
