/********************************************************************************/
/* Homework: Prog 1 (Simple Shell)						*/
/* course: CS 4560 - Operating Systems						*/
/* Author: Jacqueline Lee 							*/
/* Date: Feb. 6, 2012								*/
/********************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */

/* type entry that will store commands in history buffer */
static int n;
struct historyEntry{
     int commandNum;
     char command[MAX_LINE];	
};
static struct historyEntry hisBuffer[10];  //array of history entries (an int and string)


/**
 * setup() separates command into distinct tokens
 * using whitespace as delimiters. setup() sets the args parameter as a
 * null-terminated string.
 */
void setup(char inputBuffer[], char *args[],int *background, int *ct)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start;  /* index where beginning of next command parameter is */
    
    *ct = 0;   //number of arguments for current command
    length = strlen(inputBuffer);  //length of string to tokenize

    start = -1;
    if (length == 0)        /* ^d was entered, end of user command stream */
        return;
    if (length < 0){
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */  
    }

    /* examine every character in the inputBuffer */
    for (i=0;i<length;i++) {
        switch (inputBuffer[i]){
          case ' ':
          case '\t' :               /* argument separators */
            if(start != -1){
                    args[*ct] = &inputBuffer[start];    /* set up pointer */
                (*ct)++;
            }
            inputBuffer[i] = '\0'; /* add a null char; make a C string */
            start = -1;
            break;
          case '\n':                 /* should be the final char examined */
            if (start != -1){
                    args[*ct] = &inputBuffer[start];    
                (*ct)++;
            }
                inputBuffer[i] = '\0';
                args[*ct] = NULL; /* no more arguments to this command */
            break;
          default :             /* some other character */
            if (start == -1)
                start = i;
            if (inputBuffer[i] == '&'){
                *background  = 1;
                start = -1;
                inputBuffer[i] = '\0';
            }
          }
     }   
     args[*ct] = NULL; /* just in case the input line was > 80 */
   	
}


/**
*^C signal handler function
*prints out history buffer */
void handle_SIGINT(){
    
    char string[MAX_LINE + 10];

    /*print historyBuf*/
    int i, j;     		   //loop iterators 
    if(n < 10) i = 1;   	   //if less than 10 entries, start at 1
    if(n >= 10)i = (n +1) % 10;    //if more than or = 10 entries, 
				   // indicates that circular array looped
				   // start at entry after current(start of last 10 entries)
    
    /* iterates for 10 entries( if n > 10) or n (if less than 10 entries) */
    /* i is placed at entry to grab (circular implementation) */
    for(i, j=1; (j <= 10) && (j <= n); i = (i+1) % 10, j++){   
	sprintf(string, "\n  %d ", hisBuffer[i].commandNum);
	strcat(string, hisBuffer[i].command);
	write(STDOUT_FILENO, string, strlen(string));
    }
}



/**
* function for built in command "r 'command number'" 
*  will return the entry in hisBuffer at 'command number'
*  if no 'command number' is provided, returns the last entry */
char* r_history(char *args[], int ct){

    int arg1;   //holds integer conversion of 'command number' argument

    //if history buffer is empty return with empty string
    //which will be caught by setup()	
    if(n < 0){
	printf("The history buffer is empty.");
	return "";
    }

    /*if given a at least a second argument:
    * ignores additional arguments after second */
    if(ct >= 2){
    	arg1 = atoi(args[1]);  //convert second argument into integer
    	
	
    	//validate args[1], compare  to n
    	if(arg1 < n - 10  || arg1 < 0 || arg1 > n){
		printf("Command does not exist in history buffer");
		return "";  
    	}
        
        //grab command out of history buffer at given number and return it
        printf(" %s \n", hisBuffer[arg1 % 10]. command);
	return hisBuffer[arg1 % 10].command;
    }
    //else if command r has no arguments, return most recent command
    printf("%s \n", hisBuffer[n % 10].command);
    return  hisBuffer[(n) % 10].command;
}



/*** MAIN ***/
int main(void)
{
    char inputBuffer[MAX_LINE];  /* buffer to hold the command entered */
    int background;              /* equals 1 if a command is followed by '&' */
    char *args[(MAX_LINE/2)+1];  /* command line (of 80) has max of 40 arguments */ 	
    int ct;  			 /* counts the nmuber of arguments in command*/
   
    /* set up signal handler */
    struct sigaction handler;
    handler.sa_handler = handle_SIGINT;
    handler.sa_flags = SA_RESTART;

   
    int ppid = getpid();  //parent pid
    printf("*****************************************\n");
    printf("Welcome to jlshell! My pid is %d.\n", ppid);
    printf("*****************************************\n\n");
    fflush(stdout);       //flush out-stream buffer, needs to be here to print to screen?
   

    n = 0;		//number of commands successfully entered, declared globally
    int  np = 1;	//number of prompts
    pid_t pid;		//proccess id of child
    while (1){          /* *Program terminates normally inside setup** */
        background = 0;	//flag background as false	 	
    	sigaction(SIGINT, &handler, NULL); 
	printf("\njlshell[%d]:", np);		
        fflush(stdout);
        np++;           //increment number of prompts

	memset(inputBuffer, 0, MAX_LINE);            //clear inputBuffer       
        read(STDIN_FILENO, inputBuffer, MAX_LINE);   //read in next command                                                
        /*if built in command exit 
	* print to screen process information, exit */
        char tstring[50], tmpstr[10];
        if(strcmp("exit\n", inputBuffer)== 0){
		strcpy(tstring, "ps -o pid,ppid,pcpu,pmem,etime,user,command -p ");
		sprintf(tmpstr, " %d ",  ppid);
		strcat(tstring, tmpstr);
       		system(tstring);
		exit(0);	
	}
	
	
        /*if built in r command 
	* grabs command out of hisBuffer to be tokenized */
	if((strncmp("r ", inputBuffer, 2) == 0) ||  (strcmp("r\n", inputBuffer) == 0)){
		//isolates r and command number for us
		setup(inputBuffer, args, &background, &ct);  
		//gets next command out of history buffer and places it into inputBuffer
		strcpy(inputBuffer, r_history(args, ct));     
	}        

	/*if built in command count 
	* counts the length of the string after count */
	int length;
	if(strncmp("count ", inputBuffer, 6) == 0 ){
		length = (strlen(inputBuffer) - 7);  //7 = "count " and '\n'
		printf("%d\n", length);
	}
        
        /*copy command into history buffer */
        n++;	//increment number of commands entered
        struct historyEntry entry;  //entry to store into history buffer
        entry.commandNum = n;
        strcpy(entry.command, inputBuffer);
        hisBuffer[n % 10] = entry; 

        setup(inputBuffer,args,&background, &ct);   //gets and tokenizes next command   

	pid_t cpid; //child pid
	pid=fork(); //fork off child
	if(pid == 0){/*child proccess forked succesfully, for child: */	
		cpid = getpid();
		printf("[Child pid: %d,  Background = %s]\n" , cpid, (background)?"true":"false");
		fflush(stdout);	
		execvp(args[0],args);
		_exit(0);
	}
	else if(pid == -1){/* parent failed to fork a child */
		fprintf(stderr, "Fork Failed.");
		return 1;
	}
	else{/*in parent process*/
		if(background == 1){/*empty, skips wait()*/}
		else
		waitpid(cpid);  //parent will wait for child to terminate
	}
    }

    return 0;
}
