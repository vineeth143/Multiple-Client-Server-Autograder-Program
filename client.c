/*
Vineeth kumar balapanuru
20CS60R56
*/

// Client program
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*for getting file size using stat()*/
#include<sys/stat.h>

/*for sendfile()*/
#include<sys/sendfile.h>

/*for O_RDONLY*/
#include<fcntl.h>

//comparing two strings
int compare(char *str1,char *str2)
{
	int temp1=0,temp2=0;
	int l1=strlen(str1);
	int l2=strlen(str2);
	if(l1>l2)
		temp1=1;
	else if(l1<l2)
		temp2=1;
	int flag=1;
	int i=0,j=0;
	while(i<strlen(str1) && j<strlen(str2))
	{
		if(flag==0)
		{
			break;
		}
		else if(str1[i]==13 || str1[i]==9)
		{
			i++;
			continue;
		}
		else if(str2[i]==13 || str2[i]==9)
		{
			j++;
			continue;
		}
		else
		{
			if(str1[i]!=str2[j])
			{
				flag=0;
				break;
			}
			i++;j++;
		}
	}
	if(i==0 || j==0)
		flag=0;
	return flag;
}

int main(int argc, char const *argv[])
{
	int check;
	int sock = 0, valread;
	int PORT=atoi(argv[2]);
	struct sockaddr_in serv_addr;
	// Convert IP addresses from text to binary form
	struct hostent *server;
	server = gethostbyname(argv[1]);
	bzero((char *) &serv_addr, sizeof(serv_addr));
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	//int filehandle;

	char buffer[1024] = {0};
	sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock==-1)
  {
    printf("[-]socket creation failed\n");
    exit(1);
  }

	//Assigning address and port number
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	//Connecting to server
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n[+]Connection Failed\n");
		return -1;
	}
  else
    printf("[+]Connected to server\n");
  while(1)
  {
    memset(buffer, 0, 1024*sizeof(char));
    printf("[+]Please enter the message to the server:");
    char operation[1024]={0};
		char *command;
		char *file_name;
		char *extension;
		char *f;
		//scanf("%[^\n]%*c",operation);
		//fgets(operation, 100, stdin);
		gets(operation);
		//printf("%lu\n",strlen(operation));
		if(strlen(operation)==0)
		{
			printf("[-]Invalid command\n");
			continue;
		}
		int len=strlen(operation);
		int i,flag=0;
		for(i=0;i<len;i++)
		{
			if(operation[i]==' '){
				flag=1;
				break;
			}
		}
		if(flag==1)
		{
			command=operation;
			command[i]='\0';
			file_name=operation+(i+1);
			int fl=0,j;
			for(j=0;j<strlen(file_name);j++)
			{
				if(file_name[j]==' '){
						fl=1;
						break;
				}

			}
			if(fl==1)
			{
				extension=file_name+(j+1);
				file_name[j]='\0';

			}
		}
		else
		{
			command=operation;
		}
    if(strcmp(operation,"QUIT")==0)
    {
      write(sock , operation , 1024);
      break;
    }
    else
    {
			//printf("%s\n",command);
      if(strcmp("RETR",command)==0)
			{
        //printf("%ld\n",strlen(file_name));
				// code for retrieving a file
        int filehandle,size;
				strcpy(buffer, "RETR ");
	  		strcat(buffer, file_name);

        //checking whether the file is already present or not
        system("ls >temps.txt");
        FILE *fp;
        fp=fopen("temps.txt","r");
        char *line=NULL;
        size_t len = 0;
        ssize_t read;
        int flag=0;

        while ((read = getline(&line, &len, fp)) != -1) {
          //printf("%ld\n",strlen(line));
          if(compare(file_name,line)==1)
          {
            flag=1;
            break;
          }
        }
        fclose(fp);
        remove("temps.txt");
        if(flag==1)
        {
          printf("[+]File already present\n");
        }
        else
        {
          send(sock, buffer, 1024, 0);
  				check=recv(sock, &size, sizeof(int), 0);
					if(check==0)
					{
						printf("[-]server disconnected\n");
						break;
					}
  				//recv(sock, f, size, 0);
  				if(!size)
  	    	{
  	      	printf("[-]No such file on the remote directory\n\n");
  	    		continue;
  	    	}
  				f = malloc(size);
  				recv(sock, f, size, 0);
  				while(1)
  	    	{
  	      	filehandle = open(file_name, O_CREAT | O_EXCL | O_WRONLY, 0666);
  	      	if(filehandle == -1)
  					{
  		  			sprintf(file_name + strlen(file_name), "%d", i);//needed only if same directory is used for both server and client
  					}
  	      	else break;
  	    	}
  	  		if(write(filehandle, f, size)!=-1)
          {
            printf("[+]file retrieved successfully\n");
          }
          else
            printf("[-]error encountered\n");
  	  		close(filehandle);
          free(f);
        }
			}
			else if(strcmp("STOR",command)==0)
			{
				// code for storting a file
        struct stat	obj;
        int filehandle,size,status;
        stat(file_name, &obj);
        strcpy(buffer, "STOR ");
	  		strcat(buffer, file_name);
	  		send(sock, buffer, 1024, 0);
        filehandle = open(file_name, O_RDONLY);
        size = obj.st_size;
        if(filehandle == -1)
           size = 0;
				send(sock, &size, sizeof(int), 0);
        if(size){

          sendfile(sock, filehandle, NULL, size);
          printf("[+]sending file to the server\n");
        }
        else{
          printf("[-]no such file or directory\n");
          continue;
        }
        recv(sock, &status, sizeof(int), 0);
        if(status)
          printf("[+]File stored in server successfully\n");
        else if(status==0)
          printf("[+]File already present in server\n");
        else
          printf("File failed to be stored to remote machine\n");
			}
			else if(strcmp("LIST",command)==0)
			{
				// code for extracting list of file names
        int size,filehandle;
        char *f;
        strcpy(buffer, "LIST");
        send(sock, buffer, 1024, 0);
        check=recv(sock, &size, sizeof(int), 0);
				if(check==0)
				{
					printf("[-]server disconnected\n");
					break;
				}
        f = malloc(size);
        recv(sock, f, size, 0);
        //printf("%s\n",f);
        filehandle = creat("temp.txt", O_WRONLY);
        write(filehandle, f, size);
        close(filehandle);
        //printf("The remote directory listing is as follows:\n");
        system("cat temp.txt");
			}
			else if(strcmp("DELE",command)==0)
			{
				// code for deleting a file
        int status;
        strcpy(buffer, "DELE ");
        strcat(buffer, file_name);
        send(sock, buffer, 1024, 0);
        check=recv(sock, &status, sizeof(int), 0);
				if(check==0)
				{
					printf("[-]server disconnected\n");
					break;
				}
        if(!status)
          printf("[+]File deleted successfully\n");
        else
          printf("[-]requested file doesn't exist\n");
			}
			else if(strcmp("CODEJUD",command)==0)
			{
				//code for codejud
				//printf("%s\n",file_name);
				//storing the C/C++ file in server
				strcpy(buffer, "CODEJUD ");
        strcat(buffer, file_name);
				strcat(buffer," ");
				strcat(buffer,extension);
				send(sock, buffer, 1024, 0);
				struct stat	obj;
        int filehandle,size;
        stat(file_name, &obj);
				filehandle = open(file_name, O_RDONLY);
        size = obj.st_size;
        if(filehandle == -1)
           size = 0;
				send(sock, &size, sizeof(int), 0);
        if(size){
          sendfile(sock, filehandle, NULL, size);
					//printf("##\n");
          //printf("[+]sending file to the server\n");
        }
        else{
          printf("[-]no such C/CPP file\n");
          continue;
        }
				//receiveing the status of compilation
				int status_c;
				check=recv(sock, &status_c, sizeof(int), 0);
				if(check==0)
				{
					printf("[-]server disconnected\n");
					break;
				}
				if(status_c==-999)
				{
					printf("[-]invalid extension\n");
				}
				else if(status_c)
				{
					printf("[-]COMPILE_ERROR\n");
				}
				else
				{
					printf("[+]COMPILE_SUCCESS\n");

					//receiveing execution status
					int status_e;
					recv(sock, &status_e, sizeof(int), 0);
					if(status_e==1)
					{
						printf("[-]RUN_ERROR\n");
					}
					else if(status_e==2)
					{
						printf("[-]TIME LIMIT EXCEEDED\n");
					}
					else
					{
						printf("[+]RUN_SUCCESS\n");

						//receiveing testcases matching status
						int status_m;
						recv(sock, &status_m, sizeof(int), 0);
						if(status_m==1)
						{
							printf("[-]WRONG_ANSWER\n");
						}
						else if(status_m==-999)
						{
							printf("[-]testcases are missing\n");
						}
						else
						{
							printf("[+]ACCEPTED\n");
						}

					}

				}
			}
			else
			{
				//invalid command
				printf("[-]Invalid command\n");
			}
    }
  }

	close(sock);
	return 0;
}
