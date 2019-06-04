#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <sys/mman.h>
#include <termios.h>




#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "cs321@"

int keyPressed = 0;


int execute(char* arglist[]);
char** tokenize(char* cmdline);
char* read_cmd(char* , FILE* , int , char [10][30][30]);
char** checkIORedirection(char **);
int checkPipes(char ** , char **);
int stringLen(char **);
void handler(int );
int checkSemiColon(char **);
char*** mulCmdTokenize(char**); //used to tokenize more than two commands sepreated b ; in a 3D array
int cmdHistoryInput(char [10][30][30] , char ** , int);
void callPrevCommand(char [10][30][30] , int , int);
int checkUpDownKey(FILE* , int , char [10][30][30]);
void jobs();
int convertToInt(char*);
void help();


void printArray(char mulArglist[10][30][30]){
	for( int i = 0 ; i < 10; i++){

		for( int j = 0 ; j < ARGLEN; j++){
			if (mulArglist[i][j] == NULL){
				break;
			}
			printf("%s\t" ,mulArglist[i][j]);
		}
		printf("\n");

	}


}


//use to initalize a 3D array with '\0' character
void initialize3DArray(char mulArglist[10][30][30]){
	for( int i = 0 ; i < 10; i++){

		for( int j = 0 ; j < ARGLEN; j++){
			strcpy(mulArglist[i][j] , "\0");
		}
	}


}

/*
void printArray(char*** mulArglist){
	for( int i = 0 ; mulArglist[i][0] != NULL; i++){

		for( int j = 0 ; j < ARGLEN; j++){
			if (mulArglist[i][j] == NULL){
				break;
			}
			printf("%s\t" ,mulArglist[i][j]);
		}
		printf("\n");

	}


}*/


int main(){
	char *cmdline;
	char **arglist;
	char ***mulArglist;
	char *prompt = PROMPT;
	char cmdHistory[10][ARGLEN][ARGLEN]; //to Hold command History
	int cmdHistoryCount = 0;
	initialize3DArray(cmdHistory); //initalize 3D array

	//numOfBgProc = mmap(0 , sizeof *numOfBgProc, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1 ,0);//use to make gloabal variable access by both child and parent
	//*numOfBgProc = 0;

	signal(SIGINT, SIG_IGN); //Will ignore SIGINT in parent
	signal(SIGCHLD, handler);//will check if a child proc is finished and decrement in *numOfBgProc
	while((cmdline = read_cmd(prompt,stdin , cmdHistoryCount , cmdHistory )) != NULL){

		if((arglist = tokenize(cmdline)) != NULL){

			if(checkSemiColon(arglist)){//check if cmnd has semi colon in it
				cmdHistoryCount = cmdHistoryInput(cmdHistory , arglist , cmdHistoryCount);//call the cmdHistory Function to take command into Command History 3D array
				mulArglist = mulCmdTokenize(arglist);
				//printArray(mulArglist);	

				for( int i = 0 ; mulArglist[i][0] != NULL; i++){
					execute(mulArglist[i]);//execute multilple commands sepreated by semicolon

					

				}

				for(int i = 0 ; i < MAXARGS + 1 ; i++){
					for(int j = 0 ; j < MAXARGS + 1 ; j++){
						free(mulArglist[i][j]); //free 3D array
					}
					free(mulArglist[i]);
				}
				free(mulArglist);
				}


			else{			
				
				if(arglist[0] && arglist[0][0] == '!' && arglist[0][1] == '-'&& (!arglist[0][3] || arglist[0][3] == '0')){

					int num = strcmp(arglist[0] , "!-");
					if((num > 48 && num < 59) && cmdHistoryCount){//check if previous command is demanded
					//strcmp will return the value of the prev command number , which is subtracted from 48 further

						if(arglist[0][3] == '0'){
							num = 58;//so that if user enter !-10 it can see 10th last command

						}
	callPrevCommand(cmdHistory , cmdHistoryCount , (cmdHistoryCount)-(num-48));

					}
				}


				else if(arglist[0] && arglist[0][0] == '!' && (!arglist[0][2] || arglist[0][2] == '0')){


					int num = strcmp(arglist[0] , "!");
					
					if((num >= 48 && num < 59) && cmdHistoryCount){//check if previous command is demanded
					//strcmp will return the value of the prev command number , which is subtracted from 48 further

						if(arglist[0][2] == '0'){
							num = 58;//so that if user enter !-10 it can see 10th last command

						}
							callPrevCommand(cmdHistory , cmdHistoryCount , (num-48)-1);		

					}

			}
				else{
					execute(arglist);
					cmdHistoryCount = cmdHistoryInput(cmdHistory , arglist , cmdHistoryCount);//call the cmdHistory Function to take command into Command History 3D array
				}
			}
			// need to free arglist

			for(int j = 0 ; j < MAXARGS + 1 ; j++)
				free(arglist[j]);
			
	 		
	 		free(arglist);
			free(cmdline);
			
			}
			//printArray(cmdHistory);
		
				
	}
	//end of while loop
	printf("\n");


	//munmap(numOfBgProc , sizeof *numOfBgProc);//end conf of global variable
	return 0;
}


//function to call the previous commands
void callPrevCommand(char cmdHistory[10][30][30] , int cmdHistoryCount , int n){

	char** tempArray = (char**)malloc(sizeof(char*)*(MAXARGS+1));
			for(int i = 0 ; i < MAXARGS + 1 ; i++){
				tempArray[i] = (char*)malloc(sizeof(char*)*ARGLEN);
				bzero(tempArray[i] , ARGLEN);
			}

	if(n >= 0 && n<=cmdHistoryCount){//check if given cmnd number is in the array

		for(int i = 0 ; i < MAXARGS ; i++){//initalize the string eith \0
			strcpy(tempArray[i] , "\0");
		}

		int execLimit = 0; //to check how many commands tempArray have to put Null at the and of the array
		for(int i = 0 ; i < MAXARGS ; i++){

			if(strcmp(cmdHistory[n][i] , "\0") == 0){				
				execLimit = i;
				break;
			}

			printf("%s "  , cmdHistory[n][i]);//display the previous command
			strcpy(tempArray[i] , cmdHistory[n][i]);//copy the previous command into arglistd
		}
		tempArray[execLimit] = NULL;
		printf("\n");

		if(checkSemiColon(tempArray)){//check if cmnd has semi colon in it

			char ***tempMulArglist = (char***)malloc(sizeof(char**)*(MAXARGS+1));
			for(int i = 0 ; i < MAXARGS + 1 ; i++){
			tempMulArglist[i]= (char**)malloc(sizeof(char*)*ARGLEN);
					bzero(tempMulArglist[i] , ARGLEN);
				for(int j = 0 ; j < MAXARGS + 1 ; j++){
					tempMulArglist[i][j] = (char*)malloc(sizeof(char)*ARGLEN);
					bzero(tempMulArglist[i][j] , ARGLEN);
				}
			}




			tempMulArglist = mulCmdTokenize(tempArray);
			//printArray(tempMulArglist);	

			for( int i = 0 ; tempMulArglist[i][0] != NULL; i++){
				execute(tempMulArglist[i]);//execute multilple commands sepreated by semicolon

			}

			for(int i = 0 ; i < MAXARGS + 1 ; i++){
				for(int j = 0 ; j < MAXARGS + 1 ; j++){
					free(tempMulArglist[i][j]); //free 3D array
				}
				free(tempMulArglist[i]);
			}
			free(tempMulArglist);
			}
		else{

			execute(tempArray);
		}

	}
	else{
		printf("Command not found in previous commands\n\n");
	}
	for(int j = 0 ; j < MAXARGS + 1 ; j++)
		free(tempArray[j]);

	free(tempArray);
}





//function to put input history into a 3D array
int cmdHistoryInput(char cmdHistory[10][30][30] , char ** inputArray , int cmdHistoryCount){

	if(inputArray[0] != NULL){//check if input is enterd or not

		if(cmdHistoryCount <10){
			int a;
			for(int a = 0 ; inputArray[a] != NULL ; a++){
				strcpy(cmdHistory[cmdHistoryCount][a] , inputArray[a]);

			}
			(cmdHistory[cmdHistoryCount][a] , NULL);
			cmdHistoryCount++;
		}
		else{
			for(int i = 0 ; i < 9 ; i++){
				for(int j = 0 ; j < ARGLEN ; j++){//shift array to left
					strcpy(cmdHistory[i][j] , cmdHistory[i+1][j]); 
				}
			}


			for(int i = 0 ; i < ARGLEN ; i++){//clear the last array in 3D array to hold the new Command data
				strcpy(cmdHistory[9][i] , "\0");

			}
			
			for(int i = 0 ; inputArray[i] ; i++){//input at he last index		
				strcpy(cmdHistory[9][i] , inputArray[i]);
				
			}
		}



	}
	


	return cmdHistoryCount;

}


int execute(char * arglist[]){
//printf("a\n");
	int status , noOfPipes;


	if(strcmp(arglist[0] , "cd") == 0){
		if(arglist[1]){
			chdir(arglist[1]);
		}
	}
	else if(strcmp(arglist[0] , "exit") == 0){
		exit(0);
	}

	

	else{
		int cpid = fork();
		char** arglist_2 = (char**)malloc(sizeof(char*)*(MAXARGS+1));
				for(int i = 0 ; i < MAXARGS + 1 ; i++){
					arglist_2[i] = (char*)malloc(sizeof(char*)*ARGLEN);
					bzero(arglist_2[i] , ARGLEN);
				}

		switch(cpid){
			case -1:
				perror("Fork Failed");
				exit(1);
			case 0:
				signal(SIGINT , SIG_DFL); //SIGINT default behavior in child
				checkIORedirection(arglist); //check < and >
				if(checkPipes(arglist ,arglist_2)){//check if commad has pipe..
					//code to deal with pipes (handle single pipe only)s
					int pfds[2];
					pipe(pfds);

					if(!fork()){
						close(1);
						dup(pfds[1]);
						close(pfds[0]);
						if(strcmp(arglist[0] , "jobs") == 0){
							jobs();
}
						else{
							execvp(arglist[0] , arglist);
							perror("Command not found...");
						}
					
						exit(0);
					}
					else{
						waitpid(cpid, &status, 0);
						close(0);
						dup(pfds[0]);
						close(pfds[1]);
						if(strcmp(arglist_2[0] , "jobs") == 0){
							jobs();
						}


						else{
							execvp(arglist_2[0] , arglist_2);
							perror("Command not found...");
						}
						exit(1);
					}

				}
				//stringLen function is used to find the length of 2D char string
				if(strcmp(arglist[stringLen(arglist)],"&") == 0){
					arglist[stringLen(arglist)] = NULL;

					int bgStatus;
					pid_t bgpid = fork();
					

					if(!bgpid){

						execvp(arglist[0] , arglist);
						perror("Command not found...");
						exit(0);
					}
					else{
						printf("\t[%d]\n",bgpid);	
					exit(0);



					}
					
						
				}
				else if(strcmp(arglist[0] , "jobs") == 0){

					jobs();
					exit(0);
				}
				else if(strcmp(arglist[0] , "kill") == 0){

					kill(convertToInt(arglist[1]) , convertToInt(arglist[2]));
					exit(0);
				}
				else if(strcmp(arglist[0] , "help") == 0){

					help();
					exit(0);
				}
				else{

					execvp(arglist[0] , arglist);
					perror("Command not found...");
					exit(0);
				}

			default:
				waitpid(cpid, &status, 0);
				printf("Child exited with status %d \n" , status >> 8);

				free(arglist_2);
	//printf("b\n");	
				return 0;
			}
	}
	return 0;
}

void help(){
	printf("\n\n\n");
	printf("---------------------------Help Page------------------------\n");
	printf("cd : command will change the current directory\n");
	printf("exit : command will close the shell program\n");
	printf("jobs : command will show the background process with there PID\n");
	printf("kill : command will kill any process shown by jobs command\n \tsyntax kill <pid> <signal number>\n");
		printf("----------------------------------------------------------");
	printf("\n\n\n");




}



int convertToInt(char* arglist){
	int num = 0;
	int pid = 0;
	for(int i = 0 ; i <strlen(arglist) ; i++){
		num = arglist[i] - '0';
		pid = pid*10 + num;
	}

//	printf("pid = %d\n",pid);
	return pid;
}




void jobs(){//will run two command and join them using a pipe method to obtain jobs like result
	int pfds[2] , status;
	pipe(pfds);
	int jpid = fork();
	if(!jpid){
		close(1);
		dup(pfds[1]);
		close(pfds[0]);
		execl("/usr/bin/ps" , "ps" , "t" , '\0');

		exit(0);
	}
	else{
		waitpid(jpid, &status, 0);
		close(0);
		dup(pfds[0]);
		close(pfds[1]);
		execl("/usr/bin/grep" , "grep" , "-w" ,"T" , '\0');
		perror("Command not found...");
		
	}
}


void handler(int sig){
	pid_t pid;
	pid = wait(NULL);
}

char* read_cmd(char* prompt ,FILE* fp , int cmdHistoryCount , char cmdHistory[10][30][30]){

	//code to display current working directory (next 6 lines)
	long size;
	char *buf;
	char *ptr;
	size = pathconf(".",_PC_PATH_MAX);
	buf = (char*)malloc((size_t)size);
	ptr = getcwd(buf , (size_t)size);
	
	printf("%s%s :- " , prompt,ptr);
	int c;//input character
	int pos = 0;//position in character in cmdline
	char* cmdline = (char*) malloc(sizeof(char)*MAX_LEN);	

	/*if(checkUpDownKey(fp , cmdHistoryCount , cmdHistory)){//check up or down key press
	}
	else{*/
		while((c = getc(fp)) != EOF){
			if(c == '\n')
				break;
			cmdline[pos++] = c;
		}
		//if user press ctrl+d to exit the shell-these lines will handle it

		if(c == EOF && pos == 0)
			return NULL;
		cmdline[pos] = '\0';


	//}
	return cmdline;
}

int checkUpDownKey(FILE* fp , int cmdHistoryCount ,  char cmdHistory[10][30][30]){
		char c;
		int pos = 0;
		char* cmdline1 = (char*) malloc(sizeof(char)*MAX_LEN);
	
//start of code block to check if user press up or down arrow key in non cononical mode
		static struct termios oldt, newt;
		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON);
		tcsetattr( STDIN_FILENO , TCSANOW , &newt); 

		int i;
		int upArrow = 0; //1 if up arrow found, 0 if not found

		while((c = getc(fp)) != '\0'){
			if(c == '\n')
				break;
			cmdline1[pos++] = c;
		}

		  for (i = 0; i < 5; i++) {
		    if (cmdline1[i] == '\33') {
		      if(cmdline1[i+2] == 'A') {
			upArrow = 1;
			break;
		      }
			else if(cmdline1[i+2] == 'B') {
			upArrow = 2;
			break;
		      }
		    }
		  }
		  if (upArrow == 1) {
			if(keyPressed > 0 && keyPressed <= cmdHistoryCount ){
				keyPressed--;
				callPrevCommand(cmdHistory , cmdHistoryCount , cmdHistoryCount -keyPressed);

				return 1;
			}
		  }
		  else if (upArrow == 2) {
		    if(keyPressed <= cmdHistoryCount  && keyPressed >= 0){
				keyPressed++;
				callPrevCommand(cmdHistory , cmdHistoryCount , keyPressed);

				return 1;
			}
		  } 
		else {
		    printf("Invalid command\n");
		  }
		tcsetattr( STDIN_FILENO , TCSANOW , &oldt);
//End of code block

		printf("key pressed = %d\n" , keyPressed);


	free(cmdline1);
return 0;

}






char** tokenize(char * cmdline){
	char** arglist = (char**)malloc(sizeof(char*)*(MAXARGS+1));
	for(int i = 0 ; i < MAXARGS + 1 ; i++){
		arglist[i] = (char*)malloc(sizeof(char*)*ARGLEN);
		bzero(arglist[i] , ARGLEN);
	}
	for(int i = 0 ; i < MAXARGS ; i++){//initalize the string eith \0
		strcpy(arglist[i] , "\0");
	}

	char* cp = cmdline; // pos in string
	char* start;
	int len;
	int argnum = 0; //slot used
	while(*cp != '\0'){
		while(*cp == ' ' || *cp == '\t')//skip leading spaces
			cp++;
		start = cp;//start of the word
		len = 1;//initialize length of the word to 1
		//find the end of the word
		while(*++cp != '\0' && !(*cp == ' ' || *cp == '\t'))
			len++;
		strncpy(arglist[argnum], start, len);
		arglist[argnum][len] = '\0';
		argnum++;
	}
	arglist[argnum] = NULL;

	return arglist;
}


char** checkIORedirection(char ** arglist){
	//loop to check '>' and '<' in the arg list
	int i = 0;
	while(arglist[i]){
		struct stat buf;
		if(strcmp(arglist[i] , "<") == 0){
			printf("Input Success\n");
			i++;
			struct stat info;
			int rv = lstat(arglist[i] , &info);
			if(lstat(arglist[i] , &buf) < 0){
				perror("Error in stat");
				exit(1);
			}
			if( rv == -1){
				perror("stat failed");
				exit(1);
			}
			//check if file is a regular file
			if((buf.st_mode & 0170000) == 0100000){
				//open in read only mode and fb is pointed to value 0 in ppfdt table
				int fd = open(arglist[i] , O_RDONLY);
				dup2(fd,0);


				int count = i;
				//loop to shift the array to left in order to remove '>' from arglist
				while(arglist[i]){
					strcpy(arglist[i-1] , arglist[i]);
					i++;
				}
				arglist[i] = NULL;
				i = count-1;
			}
			else{
				printf("Not a regular file\n");
			}

		}

		else if(strcmp(arglist[i] , ">") == 0){
			i++;
			struct stat info;
			int rv = lstat(arglist[i] , &info);
			if(lstat(arglist[i] , &buf) < 0){
				perror("Error in stat");
				exit(1);
			}
			if( rv == -1){
				perror("stat failed");
				exit(1);
			}
			if((buf.st_mode & 0170000) ==  0100000){
				//open in read only mode and fb is pointed to value 0 in ppfdt table
				int fd = open(arglist[i] , O_WRONLY);
				dup2(fd,1);

				//loop to shift the array to left in order to remove '>' from arglist
				int count = i;
				while(arglist[i]){
					strcpy(arglist[i-1] , arglist[i]);
					i++;
				}
				arglist[i] = NULL;
				i = count-1;
			}
			else{
				printf("Not a regular file\n");
			}

		}
	i++;
	
	
	}

	return arglist;
}



int checkPipes(char **arglist, char **arglist_2){
	int i = 0 , a = 0;
	int check = 0;

	
	while(arglist[i]){
		if(strcmp(arglist[i],"|") == 0){
			check++ ;
			strcpy(arglist[i] , "\0");
			i++;
		}
		if(check){
			strcpy(arglist_2[a] , arglist[i]);
			strcpy(arglist[i] , "\0");
			a++;
		}
		i++;
	}

	if(!check){
		return 0;
		}

	arglist_2[a] = NULL;
	arglist[i-a-1] = NULL;
	return 1;


}



int stringLen(char ** arglist){
	int i = 0;
	while(arglist[i]){
		i++;
	}
return i-1;

}


int checkSemiColon(char ** arglist){
	int i = 0;
	while(arglist[i]){
		if(strcmp(arglist[i] , ";") == 0){
			return 1;
		}
		i++;
	}
	return 0;
}


//copy the argument sepreated by ; in a 3D array
//so that each array can be executed by passing it in a execute function
char*** mulCmdTokenize(char** arglist){
	char ***mulArglist = (char***)malloc(sizeof(char**)*(MAXARGS+1));
	for(int i = 0 ; i < MAXARGS + 1 ; i++){
	mulArglist[i]= (char**)malloc(sizeof(char*)*ARGLEN);
			bzero(mulArglist[i] , ARGLEN);
		for(int j = 0 ; j < MAXARGS + 1 ; j++){
			mulArglist[i][j] = (char*)malloc(sizeof(char)*ARGLEN);
			bzero(mulArglist[i][j] , ARGLEN);
		}
	}
	int index = 0;
	int i  = 0;
	while(arglist[index]){
		for(int j = 0 ; j < MAXARGS ; j++){
			if (!arglist[index]){
				mulArglist[i][j] = NULL;
				break;
			}
			if(strcmp(arglist[index] , ";") == 0){
				mulArglist[i][j] = NULL;
				i++;
				index++;
				break;
			}
			strcpy(mulArglist[i][j] , arglist[index]); 
			index++;
		}

	}

	mulArglist[i+1][0] = NULL;
	return mulArglist;

}





