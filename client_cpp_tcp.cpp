/* References and methodology are written on the server portion of TCP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
    int sockfd, portno, n, pos, pos2, i_contnum;
	unsigned int length;
	string input, output, packet, key, num, contnum, substr;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	char buffer[256];

	if (argc < 3) {
		fprintf(stderr,"ERROR: Inavalid number of args. Termintating\n");
		return 0;
	}
	portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        fprintf(stderr,"ERROR: Failed to open socket. Terminating\n");
		return 0;
	}
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR: Could not connect to server. Terminating\n");
		close(sockfd);
		return 0;
    }
	if(portno > 65535 || portno < 1024){
        fprintf(stderr,"ERROR: Invalid port. Terminating\n");
		close(sockfd);
		return 0;
	}
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		fprintf(stderr,"ERROR: Could not connect to server. Terminating\n");
		return 0;
	}
	else
		cout << "Connected.\n";
	i_contnum = 0;
	while(true){
		packet.clear();
		input.clear();
		key.clear();
		std::getline(cin,input);
		length = input.size();
		
		if(length > 4094000000){
			fprintf(stderr,"ERROR: Invalid command.\n");
			continue;
		}
		if(input.compare("help")==0){
			cout << "Usage Instructions:\n?key --> returns 'key=value'.\nkey=value --> stores key and value, returns OK.\nlist --> returns all 'key=value' pairs stored.\nlistc num --> returns first 'num' of 'key=value' pairs and a contnum\nlistc num contnum --> returns 'num' of 'key=value' pairs continuing from previous listc call;\n      requires previously returned contnum. Ends with a new contnum or END.\n";
			continue;
		}else if((pos = input.find_first_of("?")) == 0 ){ // ?key Command
			if((pos2 = input.find_first_of("=")) == 1){
				fprintf(stderr,"ERROR: Invalid command.\n");
				continue;
			}
			//=====same=====//
		}else if(input.compare("list") == 0){ //List Command
			;
		}else if((pos = input.find("listc")) == 0){
			if((pos2 = input.find_first_of("=")) != string::npos){
				fprintf(stderr,"Error: Invalid command.\n");
				continue;
			}
			if(input.size() > 6){ //more than just listc
				pos = input.find_first_of(" ");
				num = input.substr(pos+1);
				if((pos2 = num.find_first_of(" ")) != string::npos){
					contnum = num.substr(pos2+1);
					if(contnum.find_first_of("=") != string::npos){
						fprintf(stderr,"ERROR: Invalid command.\n");
						continue;
					}
				}
			}else{
				fprintf(stderr,"ERROR: Invalid command.\n");
				continue;
			}
		}else if((pos = input.find_first_of("=")) != string::npos){
			substr = input.substr(pos+1);
			if((pos = input.find("=")) == 0 || substr.find_first_of("=") != string::npos){
				fprintf(stderr,"Error: Invalid command.\n");
				continue;
			}
		}else if(input.compare("exit") == 0){
			close(sockfd);
			break;
		}else{
			fprintf(stderr,"ERROR: Invalid command.\n");
			continue;
		}

		n = write(sockfd,&length,sizeof(int));
		if (n < 0){
			fprintf(stderr,"ERROR: Failed to send message. Terminating.\n");
			break;
		}
		pos = 0;
		for(long int i=input.size();i > 0;i=i-255){
			if(i == 0)
				break;
			packet.clear();
			packet = input.substr(pos,255);
			pos += 255;
			if(i > 255)
				n = write(sockfd,packet.c_str(),255);
			else
				n = write(sockfd,packet.c_str(),i);
		}
		n = read(sockfd,&length,sizeof(int));
		output.clear();
		for(long int i=length; i > 0; i=i-255){
			bzero(buffer,256);
			if(i > 255)
				n = read(sockfd,buffer,255);
			else
				n = read(sockfd,buffer,i);
			if (n < 0 )
				break;
			output.append(buffer);
		}
		if(length != output.size()){
			fprintf(stderr,"ERROR: Invalid packet from server. Terminating.\n");
			break;
		}
		cout << output.c_str() << "\n";
	}
	close(sockfd);
	return 0;
}


