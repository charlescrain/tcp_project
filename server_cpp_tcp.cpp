/* References: http://www.linuxhowtos.org/C_C++/socket.htm
 * The above website provided information for TCP and UDP; there is example
 * code for both protocols. 
 *
 * Methodology: I break up the data to be sent into packets of 256 bytes. I send 
 * the total length prior to sending the packets to the receiving end knows how 
 * much to expect. When the server is waiting for a read it constantly is checking 
 * the return value to maintain operability of the server should new clients
 * attempt to connect after one has disconnected.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <list>
#include <sstream> 

using namespace std;

struct keypair{
	string key;
	string value;
};

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	string input, substr, key, value, output, num, contnum, packet;
	int n, pos, pos2, i_contnum;
	unsigned int length;
	ostringstream convert;
	list<struct keypair> keypair_list;
	list<struct keypair>::iterator it;
	struct keypair *tmp;

	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		return 0;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		fprintf(stderr,"ERROR: Could not bind port. Terminating.\n");
		return 0;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	if(portno > 65535 || portno < 1024){
        fprintf(stderr,"ERROR: Invalid port. Terminating\n");
		close(sockfd);
		return 0;
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0){
		fprintf(stderr,"ERROR: Could not bind port. Terminating.\n");
		return 0;
	}
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, 
			(struct sockaddr *) &cli_addr, 
			&clilen);
	if (newsockfd < 0){ 
		fprintf(stderr,"ERROR: Could not bind port. Terminating.\n");
		return 0;
	}

	while(true){
		length =0;
		input.clear();
		output.clear();
		if(n <= 0){
			newsockfd = accept(sockfd, 
					(struct sockaddr *) &cli_addr, 
					&clilen);
			if(newsockfd < 0)
				continue;
		}

//=========Get length and Packets========//
		while(1){
			n = recvfrom(newsockfd,&length,sizeof(length),MSG_DONTWAIT,
				(struct sockaddr *)&cli_addr,&clilen);
			if(n >= 0)
				break;
		}
		if(n == 0)
			continue;
		
		for(long int i=length; i > 0; i=i-255){
			bzero(buffer,256);
			if(i > 255)
				n = read(newsockfd,buffer,255);
			else{
				n = read(newsockfd,buffer,i);
			}
			if (n < 0 )
				break;
			input.append(buffer);
		}
		if (n < 0 ){
			fprintf(stderr,"ERROR: Failed to receive message. Terminating.\n");
			break;
		}
		if(length != input.size()){
			fprintf(stderr,"ERROR: Invalid packet from server. Terminating.\n");
			break;
		}
		
		if((pos = input.find_first_of("?")) == 0 ){ //Print ?key
			key = input.substr(pos+1);
			it = keypair_list.begin();

			while(it != keypair_list.end()){
				if(it->key.compare(input.substr(1,input.size())) == 0){
					output = it->key;
					output.append("=");
					output.append(it->value);
					break;
				}
				++it;
			}
			if(it == keypair_list.end() || keypair_list.size() == 0){
				output = key;
				output.append("=");
			}
		}else if(input.compare("list") == 0){ //List Command
			pos = 0;
			it = keypair_list.begin();
			while(it != keypair_list.end()){
				output.append(it->key);
				output.append("=");
				output.append(it->value);
				output.append("\n");
				++it;
			}
			if(it == keypair_list.end()){
				output.pop_back();
			}
			
		}else if((pos = input.find("listc")) == 0){
			num.clear();
			contnum.clear();
			output.clear();
			it = keypair_list.begin();
			pos = input.find_first_of(" ");
			num = input.substr(pos+1);
			if((pos2 = num.find_first_of(" ")) != string::npos){
				contnum = num.substr(pos2+1);
				if(atoi(contnum.c_str()) == i_contnum && atoi(contnum.c_str()) !=0){
					num = num.substr(0,pos2);
					for(int i=0; i<atoi(contnum.c_str()); i++)
						++it;
					for(int i=0; i< atoi(num.c_str()); i++){
						if((i+atoi(contnum.c_str())) >= keypair_list.size())
							break;
						output.append(it->key);
						output.append("=");
						output.append(it->value);
						output.append("\n");
						++it;
						i_contnum++;
					}
					if((atoi(num.c_str())+atoi(contnum.c_str())) >= keypair_list.size())
						output.append("END");
					else{
						convert.clear();
						convert.str("");
						convert << i_contnum;
						output.append(convert.str());
					}
				}else{
					output.append("ERROR: Invalid continuation key.");
				}
			}else{
				for(int i=0; i< atoi(num.c_str()); i++){
					if(i >= keypair_list.size())
						break;
					output.append(it->key);
					output.append("=");
					output.append(it->value);
					output.append("\n");
					++it;
					i_contnum = i+1;
				}
				if(atoi(num.c_str()) < keypair_list.size()){
					convert.clear();
					convert.str("");
					convert << i_contnum;
					output.append(convert.str());
				}else{
					i_contnum=0;
					output.append("END");
				}
			}
		}else if((pos = input.find_first_of("=")) != string::npos){ //Assignment Command
			it = keypair_list.begin();
			while(it != keypair_list.end()){
				if(it->key.compare(input.substr(0,pos)) == 0){
					it->value = input.substr(pos+1);
					break;
				}
				++it;
			}
			if(it == keypair_list.end()){
				tmp = new struct keypair;
				keypair_list.insert(it,*tmp);
				--it;
				it->key = input.substr(0,pos);
				it->value = input.substr(pos+1);
				delete tmp;
			}
			output.clear();
			output.append("OK");
		}else{
			continue;
		}

//=========Send Response to Client=========//
		length = output.size();
		n = write(newsockfd,&length,sizeof(int));
		pos=0;
		for(long int i=length; i > 0; i=i-255){
			packet.clear();
			packet = output.substr(pos,255);
			pos += 255;
			if(i > 255)
				n = write(newsockfd,packet.c_str(),255);
			else
				n = write(newsockfd,packet.c_str(),i);
		}


	}
	close(newsockfd);
	close(sockfd);
	return 0; 
}
