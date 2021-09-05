/*
Vineeth kumar balapanuru
20CS60R56
*/

//Server program
#include <unistd.h>
#include <stdio.h>
#include<math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <ctype.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#define MAX 1000
 #define TRUE 1
#define FALSE 0
#define inf 9999999


/*for getting file size using stat()*/
#include<sys/stat.h>

/*for sendfile()*/
#include<sys/sendfile.h>

/*for O_RDONLY*/
#include<fcntl.h>

//comparing two strings
int compare(char *str1,char *str2)
{
  str2++;
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
		else if(str1[i]==13 || str1[i]==9 || str1[i]==10)
		{
			i++;
			continue;
		}
		else if(str2[j]==13 || str2[j]==9 || str1[j]==10)
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
	// if(i==0 || j==0)
	// 	flag=0;
	return flag;
}

//comparing two files
int CompareTestCases(FILE *fp1,FILE *fp2)
{
  int flag=0;
  char ch1 = getc(fp1);
  char ch2 = getc(fp2);

  while(1)
  {
    if(ch1==EOF && ch2==EOF)
      break;
    else if(ch1=='\n' && ch2==EOF)
    {
      ch1 = getc(fp1);
      ch2 = getc(fp2);
      continue;
    }
    else if(ch1==EOF && ch2=='\n')
    {
      ch1 = getc(fp1);
      ch2 = getc(fp2);
      continue;
    }
    else if(ch1 != ch2)
    {
      flag=1;
    }
    //output testcases have an extra newline character
    //hence excluding that while comparing


    ch1 = getc(fp1);
    ch2 = getc(fp2);
  }
  return flag;
}

int main(int argc, char const *argv[])
{
  //struct stat obj;
  time_t start, end;
  float cpu_time_used;
	int master_socket, new_socket, valread, addrlen, client_socket[20], max_clients=20, activity, i, sd, max_sd;
	int PORT=atoi(argv[1]);
	struct sockaddr_in address;
  fd_set readfds;
	int opt = TRUE;
  //int size;
  pid_t childpid;
  //int filehandle;
	addrlen = sizeof(address);
	char buffer[1024] = {0};
  float temp_clock[20];
  for(i=0;i<20;i++)
    temp_clock[i]=0.0;
	// char *hello = "Heyy from server!!!";

  for(i=0;i<max_clients;i++)
  {
    client_socket[i]=0;
  }

	//Creating a socket
  if((master_socket = socket(AF_INET, SOCK_STREAM, 0))==0)
  {
    printf("[+]Error in creating master_socket\n");
    exit(1);
  }

  //setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
  if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	// Assigning address and port number
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	// Binding socket to address and port number
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		printf("[+]Binding Error\n");
		exit(EXIT_FAILURE);
	}
  //Listening for connections
  int k=listen(master_socket, 3);
  if(k<0)
  {
    printf("[-]listening error\n");
    exit(1);
  }
  else{
    printf("[+]started listening\n");
  }
  printf("[+]waiting for connections\n");


	while(TRUE)
	{
    FD_ZERO(&readfds);

    FD_SET(master_socket, &readfds);
    max_sd=master_socket;

    for(i=0;i<max_clients;i++)
    {
      sd=client_socket[i];

      if(sd>0)
        FD_SET(sd, &readfds);

      if(sd>max_sd)
        max_sd=sd;
    }

    //waiting for an activity
    activity=select(max_sd+1, &readfds, NULL, NULL, NULL);


    //if something happend at master_socket
    if(FD_ISSET(master_socket, &readfds))
    {
      //accepting a new connection
      new_socket=accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);

      printf("[+]Connected with client socket number : %s:%d\n",inet_ntoa(address.sin_addr),ntohs(address.sin_port));

      for(i=0;i<max_clients;i++)
      {
        if(client_socket[i]==0)
        {
          client_socket[i]=new_socket;
          break;
        }
      }

    }

  for(i=0;i<max_clients;i++)
  {
    sd=client_socket[i];
		memset(buffer, 0, 1024*sizeof(char));
    if(FD_ISSET(sd, &readfds))
    {
      time(&start);
      valread=read(sd, buffer, 1024);
      if(strcmp(buffer,"QUIT")==0)
      {
        getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
        printf("[+]Disconnected from client socket number : %s:%d\n",inet_ntoa(address.sin_addr),ntohs(address.sin_port));
        close(sd);
        client_socket[i]=0;
        temp_clock[i]=0.0;
      }
      else
      {
        char *command;
    		char *file_name;
        char *extension;
    		int len=strlen(buffer);
    		int flag=0;
        //int fl=0,j;
    		for(i=0;i<len;i++)
    		{
    			if(buffer[i]==' '){
    				flag=1;
    				break;
    			}
    		}
    		if(flag==1)
    		{
    			command=buffer;
    			command[i]='\0';
    			file_name=buffer+(i+1);
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
            //printf("%s\n",command);
    			}
    		}
    		else
    		{
    			command=buffer;
    		}
        //printf("%s %s\n",command,file_name);
        if(strcmp("RETR",command)==0)
  			{
          printf("[+]client socket number %s:%d sent message: %s %s\n",inet_ntoa(address.sin_addr),ntohs(address.sin_port),buffer,file_name);
          struct stat	obj;
	        int filehandle,size;

  				// code for retrieving a file
          stat(file_name, &obj);
          filehandle = open(file_name, O_RDONLY);
	        size = obj.st_size;
	        if(filehandle == -1)
	           size = 0;
	        send(sd, &size, sizeof(int), 0);
	        if(size){
            sendfile(sd, filehandle, NULL, size);
            printf("[+]sending reply\n");
          }
          else{
            printf("[+]sending reply\n");
          }

  			}
  			else if(strcmp("STOR",command)==0)
  			{
  				// code for storting a file
          //printf("##########################\n");
          printf("[+]client socket number %s:%d sent message: %s %s\n",inet_ntoa(address.sin_addr),ntohs(address.sin_port),buffer,file_name);

          //checking whether file is already present or not
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
              //printf("%s %s\n",line,file_name);
              flag=1;
              break;
            }
          }
          fclose(fp);
          remove("temps.txt");
          if(flag==1)
          {
            int c=0;
            printf("[+]File already present\n");
            printf("[+]sending reply\n");
            send(sd, &c, sizeof(int), 0);
          }
          else
          {
            int filehandle,size;
            char *f;
            recv(sd, &size, sizeof(int), 0);
            if(!size)
              break;
            f = malloc(size);
            recv(sd, f, size, 0);
            while(1)
            {
              filehandle = open(file_name, O_CREAT | O_EXCL | O_WRONLY, 0666);
              if(filehandle == -1)
              {
                sprintf(file_name + strlen(file_name), "%d", i);//needed only if same directory is used for both server and client
              }
              else break;
            }
            int c=write(filehandle, f, size);
            if(c!=-1)
            {
              printf("[+]file stored successfully\n");
              printf("[+]sending reply\n");
            }
            else
              printf("[-]error encountered\n");
            send(sd, &c, sizeof(int), 0);
            close(filehandle);
            free(f);
          }
  			}
  			else if(strcmp("LIST",command)==0)
  			{
  				// code for extracting list of file names
          printf("[+]client socket number %s:%d sent message: %s\n",inet_ntoa(address.sin_addr),ntohs(address.sin_port),command);
          int filehandle,size;
          struct stat	obj;
          system("ls >temps.txt");
          //stores the list of all files in servers directory in a file named temps.txt
          //Hence a additional file temps.txt will be displayed
          stat("temps.txt",&obj);
          size = obj.st_size;
          printf("[+]sending reply\n");
          send(sd, &size, sizeof(int),0);
          filehandle = open("temps.txt", O_RDONLY);
          sendfile(sd,filehandle,NULL,size);
          close(filehandle);
          remove("temps.txt");
  			}
  			else if(strcmp("DELE",command)==0)
  			{
  				// code for deleting a file
          printf("[+]client socket number %s:%d sent message: %s %s\n",inet_ntoa(address.sin_addr),ntohs(address.sin_port),command,file_name);
          int c=remove(file_name);
          if(c==0)
          {
            printf("[+]file removed successfully\n");
            printf("[+]sending reply\n");
            send(sd, &c, sizeof(int),0);
          }
          else
          {
            send(sd, &c, sizeof(int),0);
            printf("[-]requested file doesn't exist\n");
            printf("[+]sending reply\n");
          }
  			}
        else if(strcmp("CODEJUD",command)==0)
        {
          printf("[+]client socket number %s:%d sent message: %s %s %s\n",inet_ntoa(address.sin_addr),ntohs(address.sin_port),command,file_name,extension);
          //printf("###\n");
          //code for codejud
          //storing the C/C++ file
          int filehandle,size;
          char *f;
          recv(sd, &size, sizeof(int), 0);
          if(!size)
            break;
          f = malloc(size);
          recv(sd, f, size, 0);
          while(1)
          {
            filehandle = open(file_name, O_CREAT | O_EXCL | O_WRONLY, 0666);
            if(filehandle == -1)
            {
              sprintf(file_name + strlen(file_name), "%d", i);//needed only if same directory is used for both server and client
            }
            else break;
          }
          int c=write(filehandle, f, size);
          if(c!=-1)
          {
            //printf("[+]file stored successfully\n");
            //printf("[+]sending reply\n");
          }
          else
            printf("[-]error encountered\n");
          //send(sd, &c, sizeof(int), 0);
          close(filehandle);
          free(f);

          //printf("%s\n",extension);
          //compilation phase
          if(strcmp(extension,"C")==0)
          {
            //code for compilation of C files
            char temp[100]={0};
            strcpy(temp,file_name);
            temp[strlen(file_name)-2]='\0';
            char src_cmd[500]={0};
            sprintf(src_cmd,"gcc %s -o %s",file_name,temp);
            //printf("%s\n",src_cmd);
            int status_c=system(src_cmd);
            //printf("%d\n",k);
            //sending the status of compilation
            printf("[+]sending the status of compilation\n");
            send(sd, &status_c, sizeof(int), 0);
            if(status_c!=0)
              continue;

            //code for execution of C files
            FILE *fp_inp;
            char inp_file[200]={0};
            sprintf(inp_file,"input_%s.txt",temp);
            fp_inp=fopen(inp_file,"r");

            //no input file needed
            if(fp_inp==NULL)
            {
              FILE *fp_out;
              char out_file[200]={0};
              sprintf(out_file,"output_%s.txt",temp);
              fp_out=fopen(out_file,"w");
              int temp_flag=0;
              time_t start, end;
              float cpu_time_used;
              time(&start);
              char exec_cmd[520]={0};
              sprintf(exec_cmd,"./%s 1>>%s",temp,out_file);
              int k=system(exec_cmd);

              if(k!=0)
                temp_flag=1;

              time(&end);
              cpu_time_used = (float) (end - start);
              //printf("%f\n",cpu_time_used);
              if(cpu_time_used>1.0)
                  temp_flag=2;

              //sending the status of execution
              printf("[+]sending the status of execution\n");
              send(sd, &temp_flag, sizeof(int), 0);
            }
            //input file needed
            else
            {
              char *line=NULL;
              size_t len = 0;
              ssize_t read;
              FILE *fp_out;
              char out_file[200]={0};
              sprintf(out_file,"output_%s.txt",temp);
              fp_out=fopen(out_file,"w");

              //printf("###\n");
              int temp_flag=0;
              time_t start, end;
              float cpu_time_used;
              while ((read = getline(&line, &len, fp_inp)) != -1) {
                FILE *fp1;
                char temp_file[108]={0};
                sprintf(temp_file,"inp_%s.txt",temp);
                fp1=fopen(temp_file,"w");
                //printf("%s",line);
                fprintf(fp1,"%s",line);
                fclose(fp1);
                char exec_cmd[520]={0};
                sprintf(exec_cmd,"./%s 0<%s 1>>%s",temp,temp_file,out_file);
                //printf("%s\n",exec_cmd);
                time(&start);
                int k=system(exec_cmd);
                time(&end);
                cpu_time_used = (float) (end - start);
                if(cpu_time_used>1.0)
                  temp_flag=2;

                if(k!=0)
                  temp_flag=1;

              }

              //printf("%f\n",cpu_time_used);

              fclose(fp_inp);
              fclose(fp_out);

              //sending the status of execution
              printf("[+]sending the status of execution\n");
              send(sd, &temp_flag, sizeof(int), 0);
            }

            //code for matching testcases and output
            char test_file[200]={0};
            sprintf(test_file,"testcase_%s.txt",temp);

            char out_file[200]={0};
            sprintf(out_file,"output_%s.txt",temp);

            FILE *fp1=fopen(out_file,"r");
            FILE *fp2=fopen(test_file,"r");
            if(fp2==NULL)
            {
              int t=-999;
              printf("[-]missing files\n");
              send(sd, &t, sizeof(int), 0);
              remove(out_file);
              remove(file_name);
              remove(temp);
              //remove(temp_file);
              continue;
            }

            //int status_m=CompareTestCases(fp1,fp2);
            int status_m;

            //printf("%d\n",status_m);
            //sending status of comparing Comparing testcases
            char com_cmd[450]={0};
            sprintf(com_cmd,"diff -b  %s  %s 1>logfile.txt",out_file,test_file);
            system(com_cmd);

            FILE *fl=fopen("logfile.txt","r");
            if (NULL != fl) {
              fseek (fl, 0, SEEK_END);
              size = ftell(fl);
              if (0 == size) {
                status_m=0;
              }
              else{
                status_m=1;
              }
            }
            remove("logfile.txt");
            printf("[+]sending the status of matching testcases\n");
            send(sd, &status_m, sizeof(int), 0);
            char temp_file[108]={0};
            sprintf(temp_file,"inp_%s.txt",temp);
            //printf("%s\n",temp_file);

            //printf("%s\n",com_cmd);

            //removing all files related to previously ran program
            // remove(test_file);
            remove(out_file);
            remove(file_name);
            remove(temp);
            remove(temp_file);

          }
          else if(strcmp(extension,"CPP")==0)
          {
            //code for compilation of C++ files
            char temp[100]={0};
            strcpy(temp,file_name);
            temp[strlen(file_name)-4]='\0';
            //printf("%s %s\n",file_name,temp);
            char src_cmd[500]={0};
            sprintf(src_cmd,"g++ %s -o %s",file_name,temp);
            //printf("%s\n",src_cmd);
            int status_c=system(src_cmd);
            //printf("%d\n",k);
            //sending the status of compilation
            printf("[+]sending the status of compilation\n");
            send(sd, &status_c, sizeof(int), 0);
            if(status_c!=0)
              continue;

            //code for execution of C files
            FILE *fp_inp;
            char inp_file[200]={0};
            sprintf(inp_file,"input_%s.txt",temp);
            fp_inp=fopen(inp_file,"r");

            //no input file is needed
            if(fp_inp==NULL)
            {
              FILE *fp_out;
              char out_file[200]={0};
              sprintf(out_file,"output_%s.txt",temp);
              fp_out=fopen(out_file,"w");
              int temp_flag=0;
              time_t start, end;
              float cpu_time_used;
              time(&start);
              char exec_cmd[520]={0};
              sprintf(exec_cmd,"./%s 1>>%s",temp,out_file);
              int k=system(exec_cmd);

              if(k!=0)
                temp_flag=1;

              time(&end);
              cpu_time_used = (float) (end - start);
              //printf("%f\n",cpu_time_used);
              if(cpu_time_used>1.0)
                  temp_flag=2;

              //sending the status of execution
              printf("[+]sending the status of execution\n");
              send(sd, &temp_flag, sizeof(int), 0);

            }

            //input file is needed
            else
            {
              char *line=NULL;
              size_t len = 0;
              ssize_t read;
              FILE *fp_out;
              char out_file[200]={0};
              sprintf(out_file,"output_%s.txt",temp);
              fp_out=fopen(out_file,"w");

              //printf("###\n");
              int temp_flag=0;
              time_t start, end;
              float cpu_time_used;
              time(&start);
              while ((read = getline(&line, &len, fp_inp)) != -1) {
                FILE *fp1;
                char temp_file[108]={0};
                sprintf(temp_file,"inp_%s.txt",temp);
                fp1=fopen(temp_file,"w");
                //printf("%s",line);
                fprintf(fp1,"%s",line);
                fclose(fp1);
                char exec_cmd[520]={0};
                sprintf(exec_cmd,"./%s 0<%s 1>>%s",temp,temp_file,out_file);
                //printf("%s\n",exec_cmd);
                int k=system(exec_cmd);

                if(k!=0)
                  temp_flag=1;

              }
              time(&end);
              cpu_time_used = (float) (end - start);
              //printf("%f\n",cpu_time_used);
              if(cpu_time_used>1.0)
                  temp_flag=2;
              fclose(fp_inp);
              fclose(fp_out);

              //sending the status of execution
              printf("[+]sending the status of execution\n");
              send(sd, &temp_flag, sizeof(int), 0);
            }

            //code for matching testcases and output
            char test_file[200]={0};
            sprintf(test_file,"testcase_%s.txt",temp);

            char out_file[200]={0};
            sprintf(out_file,"output_%s.txt",temp);

            FILE *fp1=fopen(out_file,"r");
            FILE *fp2=fopen(test_file,"r");

            if(fp2==NULL)
            {
              int t=-999;
              printf("[-]missing files\n");
              send(sd, &t, sizeof(int), 0);
              remove(out_file);
              remove(file_name);
              remove(temp);
              //remove(temp_file);
              continue;
            }

            //int status_m=CompareTestCases(fp1,fp2);
            int status_m;
            char com_cmd[450]={0};
            sprintf(com_cmd,"diff -b  %s  %s 1>>logfile.txt",out_file,test_file);
            system(com_cmd);
            fclose(fp1);
            fclose(fp2);

            FILE *fl=fopen("logfile.txt","r");
            if (NULL != fl) {
              fseek (fl, 0, SEEK_END);
              size = ftell(fl);
              if (0 == size) {
                status_m=0;
              }
              else{
                status_m=1;
              }
            }
            remove("logfile.txt");
            //sending status of comparing Comparing testcases
            printf("[+]sending the status of matching testcases\n");
            send(sd, &status_m, sizeof(int), 0);

            char temp_file[108]={0};
            sprintf(temp_file,"inp_%s.txt",temp);
            //printf("%s\n",temp_file);

            //removing all files related to previously ran program
            //remove(test_file);
            remove(out_file);
            remove(file_name);
            remove(temp);
            remove(temp_file);
          }
          else
          {
            int c=-999;
            send(sd, &c, sizeof(int), 0);
            printf("[-]invalid extension\n");
          }
        }
      }
    }
  }
}
  close(master_socket);
	return 0;
}
