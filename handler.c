
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

char* encoder(char str[]);
char* decoder(char str[]);


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256],filename[200];
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     if (argc < 2) 
         {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);  }

     fprintf(stdout, "Run client by providing host and port\n");
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (sockfd < 0) 
        error("ERROR opening socket");

     bzero((char *) &serv_addr, sizeof(serv_addr)); //turn it to zero
     portno = atoi(argv[1]);                       // convert the fist argument to integer
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
        error("ERROR on accept");

   char start[200]="\nConnection established.\n";
     write(newsockfd,encoder(start),strlen(encoder(start)));


   
   while (1){ 
        
        bzero(buffer,256);printf("\nWaiting for response...\n");
     n = read(newsockfd,buffer,255);  if (n < 0) error("ERROR reading from socket");
   
   
        
     printf("\nMessage: %s\n",buffer);printf("Decoding message...\n");sleep(1);
    printf("Decoded Message: %s\n",decoder(buffer));

    if(strncmp(decoder(buffer),"exit",4) == 0) break;

      printf("\nPlease send a message or type exit to quit\n");
      bzero(buffer,256);
     printf("$$ ");
        
        fgets(buffer,255,stdin);printf("Encoding message...\n");sleep(1);
    printf("Sending encoded message: %s",encoder(buffer));sleep(1);
        n = write(newsockfd,encoder(buffer),strlen(encoder(buffer)));  if (n < 0)  error("ERROR writing to socket");

      }     


      printf("\nsending exit signal...\n");
      n = write(newsockfd,encoder("exit"),strlen(encoder("exit")));

     close(newsockfd);
     close(sockfd);
     return 0; 
}

char* encoder(char input[])
{   
    int len = ((int)strlen(input));
    //Establish our character set
    char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    //Resulting string
    char *result = (char *) malloc(sizeof(char) * 1000);
    int index, bits = 0, pad = 0, process = 0, count = 0, temp;
    //Variables for loops
    int i, j, k = 0;
    //Take 3 characters at a time and store in process
    for (i = 0; i < len; i += 3)
    {
        //Set variables back to 0
        process = 0;
        count = 0;
        bits = 0;
        for (j = i; j < len && j <= i + 2; j++)
        {
            //Stores binary data in process
            process = process << 8;
            //Stores character in process
            process = process | input[j];
            //Incrmement count
            count++;
        }
        bits = count * 8;
        //Calculate the '=' amount
        pad = bits % 3;
        // Process 6 bits at a time from process and find value at each block
        while (bits != 0)
        {
            if (bits >= 6)
            {
                temp = bits - 6;
                index = (process >> temp) & 63;
                bits -= 6;
            }
            else
            {
                temp = 6 - bits;
                // append zeros to right if bits are less than 6
                index = (process << temp) & 63;
                bits = 0;
            }
            result[k++] = characters[index];
        }
    }
    //Add '='
    for (i = 1; i <= pad; i++)
    {
        result[k++] = '=';
    }
    //Null terminate
    result[k] = '\0';
    
    return result;
}

char* decoder(char input[])
{
    int len = ((int)strlen(input));
    //Resulting string
    char *result = (char *) malloc(sizeof(char) * 1000);
    
    //loop varibles, store the bitstream, and count the bits
    int i, j, k = 0, bitstream=0, count=0;
    
    //Process 4 characters at a time
    for (i = 0; i < len; i += 4) {
        //Reset our variables
        bitstream = 0;
        count = 0;
        for (j = 0; j < 4; j++) {
            //Store 6 bits
            if (input[i + j] != '=') {
                bitstream = bitstream << 6;
                count += 6;
            }
            
            //Find each encoded character and store in the bitstream

            //Capital letters
            if (input[i + j] >= 'A' && input[i + j] <= 'Z')
                bitstream = bitstream | (input[i + j] - 'A');
            
            //Lowercase letters
            else if (input[i + j] >= 'a' && input[i + j] <= 'z')
                bitstream = bitstream | (input[i + j] - 'a' + 26);
            
            //Numbers
            else if (input[i + j] >= '0' && input[i + j] <= '9')
                bitstream = bitstream | (input[i + j] - '0' + 52);
            
            //+
            else if (input[i + j] == '+')
                bitstream = bitstream | 62;
            
            ///
            else if (input[i + j] == '/')
                bitstream = bitstream | 63;
            
            //If we find '=' make sure to delete it
            else {
                bitstream = bitstream >> 2;
                count -= 2;
            }
        }
        
        //Store the result into result
        while (count != 0) {
            count -= 8;
            result[k++] = (bitstream >> count) & 255;
        }
    }
    
    //Null terminate
    result[k] = '\0';


    return result;
}