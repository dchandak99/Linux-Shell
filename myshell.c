#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include<setjmp.h> 
#include <string.h>
#include <unistd.h>
#include <stdbool.h> 
#include<sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

jmp_buf buf; 
int ctrl_C = 0; // true if SIGINT was encountered
int has_inp = 0;
/* Splits the string by space and returns the array of tokens
*
*/

char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

/* Signal Handler for SIGINT */
void sigintHandler(int sig_num) 
{ 
    /* Reset handler to catch SIGINT next time. 
       Refer http://en.cppreference.com/w/c/program/signal */
    signal(SIGINT, sigintHandler); 
    ctrl_C = 1;
    //printf("Cannot be terminated using Ctrl+C \n");
    printf("\n");
    if(has_inp == 0)
    {	
    	//printf("$ ") ;
    	ctrl_C = 0;
    }
    fflush(stdout); 
    // Jump to the point setup by setjmp 
    //longjmp(buf, 1); 
    //https://www.geeksforgeeks.org/g-fact22-concept-of-setjump-and-longjump/
} 

int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;

	FILE* fp;
	if(argc == 2) {
		fp = fopen(argv[1],"r");
		if(fp < 0) {
			printf("File doesn't exists.");
			return -1;
		}
	}

	int *background = (int *)malloc(MAX_TOKEN_SIZE * sizeof(int));
	int *new_background = (int *)malloc(MAX_TOKEN_SIZE * sizeof(int));
	/*char ***tokens_ser = (char ***)malloc(MAX_NUM_TOKENS * sizeof(char **));
	for(i = 0; i < MAX_NUM_TOKENS; ++i)
		tokens_ser[i] = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));

	char ***tokens_par = (char ***)malloc(MAX_NUM_TOKENS * sizeof(char **));
	for(i = 0; i < MAX_NUM_TOKENS; ++i)
		tokens_par[i] = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
*/
	int num_back = 0;

	signal(SIGINT, sigintHandler); 		// not break on ctrl+c
	//signal(SIGINT, SIG_IGN);
	
	while(1) {			
		// check for background jobs
		int new_num_back = 0;
		has_inp = 0;

		for(i=0; i<num_back; ++i)
		{	
			int stat;
			pid_t cpid = waitpid(background[i], &stat, WNOHANG); 
        	if (cpid == background[i]) 	//==0 means not exited
            {	
            	printf("Shell: Background process finished\n");
     				
        	}
        	else if(cpid == 0)	// add still in background
        	{
        		new_background[new_num_back] = background[i];
        		++new_num_back;
        	}
		}
		//remove the ones done with in new background list
		memcpy(background, new_background, MAX_TOKEN_SIZE * sizeof(int));
		num_back = new_num_back;


		// if not cd then builtin commands with all diff modes
		//char  **tokens;   
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		if(argc == 2) { // batch mode
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	
			}
			line[strlen(line) - 1] = '\0';
		} else { // interactive mode
			printf("$ ");

			/*fflush(NULL);
			
			if(ctrl_C == 1)	// for 0 job case place here
			{	
				ctrl_C = 0;	
				continue;
			}
			*/
			scanf("%[^\n]", line);
			getchar();
		}
		//debug output::printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		if(strlen(line) == 0)
			continue;	// empty command move to next command// prompt again

		has_inp = 1;
		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);
   		//printf("%d", i);
       //do whatever you want with the commands, here we just print them

		if(strcmp("cd", tokens[0]) == 0)
		{	
			//printf("%s", tokens[1]);
			if(tokens[1] == NULL)	// only cd 
			{
				printf("Shell: Incorrect command\n");
			}	//error;
			else if(tokens[2] != NULL)	// more than 2 arguments after cd
			{
				printf("Shell: Incorrect command\n");
			}	//error;

			else if(chdir(tokens[1]) < 0)
			{
				printf("Shell: Incorrect command\n");
			}
			// implement cd
			// print Shell: Incorrect command in case of incorrect format of cd command


			continue; //next line
		}
		// exit
		if(strcmp("exit", tokens[0]) == 0)
		{	
			if(tokens[1] != NULL)	// more than 1 argument after exit
			{
				printf("Shell: Incorrect command\n");
				continue;
			}	//error;
			// kill all background processes and free all memory
			for(i=0; i<num_back; ++i)
			{	
				// KILL this process
				printf("Shell: Background process finished\n");
				kill(background[i], SIGINT);
			}

			for(i = 0; tokens[i]!=NULL; i++){
				free(tokens[i]);
			}
			free(tokens);
			free(background);
			free(new_background);
			exit(0);
		}
		// if not cd then builtin commands with all diff modes
		/*if(ctrl_C == 1)
		{	
			printf("Hey! What happened ?\n");
			ctrl_C = 0;		// for future
		}*/

		int back = 0;	// not background
		for(i = 0; tokens[i] != NULL; i++);
		//printf("%d", back);
		if(strcmp("&", tokens[i-1]) == 0)
		{
			back = 1; // it is a background process
			tokens[i-1] = NULL;	// remove & from it
			free(tokens[i]);
			//printf("%d", back);
		}
		int series = 0, parallel = 0;
		// check for series -- go on pushing components in tokens_ser
		int num_series = 0;
		int j = 0;
		// store tokens[i] in tokens
		/*for(i = 0; tokens[i]!=NULL;++i)
			tokens_ser[0][j++] = tokens[i];
		//tokens_ser[0] = tokens;	// hardcode run check for 1 command
		++num_series;*/

		char ***tokens_ser = (char ***)malloc(MAX_NUM_TOKENS * sizeof(char **));
		/*for(i = 0; i < MAX_NUM_TOKENS; ++i)
		{	
			tokens_ser[i] = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
		}*/

		char ***tokens_par = (char ***)malloc(MAX_NUM_TOKENS * sizeof(char **));
		// for(i = 0; i < MAX_NUM_TOKENS; ++i)
		// 	tokens_par[i] = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));

		bool flag = 1;
		for(i = 0; tokens[i] != NULL; ++i)
		{	
			//printf("%d", i);
			if(strcmp("&&", tokens[i]) == 0)
			{	
				tokens_ser[num_series][j] = NULL;
				++num_series;
				j = 0;
				series = 1;
				flag = 1;
				continue;
			}
			if(flag == 1)
			{
				tokens_ser[num_series] = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
			}
			flag= 0;
			tokens_ser[num_series][j] = tokens[i];
			//tokens_ser[num_series][j] = strdup(tokens[i]);

			//tokens_ser[num_series][j] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
			//strcpy(tokens_ser[num_series][j], tokens[i]);
			++j;
		}
		tokens_ser[num_series][j] = NULL;
		// to read last one 

		// check for ||
		int num_parallel = 0;
		j = 0;
		flag = 1;
		// look for parallel
		for(i = 0; tokens[i] != NULL; ++i)
		{	
			if(strcmp("&&&", tokens[i]) == 0)
			{	
				tokens_par[num_parallel][j] = NULL;
				++num_parallel;
				j = 0;
				parallel = 1;
				flag = 1;
				continue;
			}
			if(flag == 1)
			{
				tokens_par[num_parallel] = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
			}
			flag = 0;
			tokens_par[num_parallel][j] = tokens[i];
			//tokens_par[num_parallel][j] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
			//strcpy(tokens_par[num_parallel][j], tokens[i]);
			//tokens_par[num_parallel][j] = strdup(tokens[i]);
			++j;
		}
		//printf("\n%d\n", j);
		tokens_par[num_parallel][j] = NULL;
		
		//printf("%d", series);
		if(parallel==0 && series==0)	// only 1 command
		{	
			//printf("hi\n");
			num_series = 1;
			series = 1;
			//clear parallel
			/*for(int j = 0; tokens_par[0][j] != NULL; ++j)
				free(tokens_par[0][j]);

			free(tokens_par[0]);*/
		}
		else if(parallel == 0)	// last command not counted in num_series
		{	
			//printf("hi1\n");
			num_series++;
			//clear parallel
			/*for(int j = 0; tokens_par[0][j] != NULL; ++j)
				free(tokens_par[0][j]);

			free(tokens_par[0]);*/
		}
		else if(series == 0)	// last command not counted in num_parallel
		{	
			//printf("hi2\n");
			num_parallel++;
			//clear series
			/*for(int j = 0; tokens_ser[0][j] != NULL; ++j)
				free(tokens_ser[0][j]);

			free(tokens_ser[0]);*/
		}
		/*for(i = 0; tokens[i] != NULL; i++){
			free(tokens[i]);
		}
		free(tokens);*/
		//printf("%d", series);
		// series may have series or 1 background
		// add: ignore subsequent processes in series when sigint comes
		//printf("\n%d\n", num_series);
		for(int ser = 0; ser < num_series; ++ser){
		//	printf("ctrl+c is %d\n", ctrl_C);
			if(ctrl_C == 1)
			{	
				ctrl_C = 0;	
				break;
			}
			//char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));

			//memcpy(tokens, tokens_ser[ser], sizeof(tokens_ser[ser]));	
			tokens = tokens_ser[ser];			
		int rc = fork();
		if(rc < 0)	// failed to fork
		{
			printf("Failed to fork: moving to next command\n");
			exit(0);
		}
		
		else if(rc == 0) // child
		{	
			if(back == 1)	// background child
			{
				if (setpgid(0, 0) < 0) 	// create own process group
                {
                	printf("Error: Setpgid\n");
                	_exit(1);
            	}		
			}
			else	// foreground
			{
				signal(SIGINT, SIG_DFL);
			}
			execvp(tokens[0], tokens);
				// error in executing
				//https://stackoverflow.com/questions/53315081/how-child-processes-get-terminated-when-use-execve-loader-in-c
			printf("Error in executing %s (moving to next command)\n", tokens[0]);
			for(i = 0; tokens[i] != NULL; i++){
				free(tokens[i]);
			}
			free(tokens);
			_exit(1);		// reap dead children when fail to exec
			//exit(0);
		}
		else
		{	
			if(back == 1)
			{
				background[num_back] = rc;			//record in list of background jobs
				num_back++;
			}
			else
			{	
				//wait(NULL);				
				waitpid(rc, NULL, 0);		//wait pid of child
				//https://stackoverflow.com/questions/22092088/the-waitpid-parameters
			}
		}

		/*
		for(i = 0; tokens[i] != NULL; i++){
			printf("found token %s (remove this debug output later)\n", tokens[i]);
		}*/
       	// check for background jobs
		/*int new_num_back = 0;

		for(i=0; i<num_back; ++i)
		{	
			int stat;
			pid_t cpid = waitpid(background[i], &stat, WNOHANG); 
        	if (cpid == background[i]) 	//==0 means not exited
            {	
            	printf("Shell: Background process finished\n");
     				
        	}
        	else if(cpid == 0)	// add still in background
        	{
        		new_background[new_num_back] = background[i];
        		++new_num_back;
        	}
		}
		//remove the ones done with in new background list
		background = new_background;
		num_back = new_num_back;*/

		// Freeing the allocated memory	
		/*for(i = 0; i<MAX_NUM_TOKENS; i++){
			free(tokens[i]);
		}
		free(tokens);*/
		//printf("ctrl+c is %d\n", ctrl_C);
			if(ctrl_C == 1)	// for 1 job case place here
			{	
				ctrl_C = 0;	
				break;
			}
	}	// for series for loop
	/*
		for(i = 0; i<num_series; i++){
			free(tokens_ser[i]);
		}
		free(tokens_ser);*/
		/*for(i = 0; tokens[i]!=NULL; i++){
			free(tokens[i]);
		}
		free(tokens);*/
		// now || case // only parallel no background
		int lead = 0;
		int *par_procs = (int *)malloc(MAX_TOKEN_SIZE * sizeof(int)); // store PIDs of parralel processes

		for(int par = 0; par < num_parallel; ++par){
			tokens = tokens_par[par];			
		int rc = fork();
		if(rc < 0)	// failed to fork
		{
			printf("Failed to fork: moving to next command\n");
			exit(0);
		}
		
		else if(rc == 0) // child
		{	
			signal(SIGINT, SIG_DFL);
			/*if(par == 0)				// 1st parallel one
			{	
				lead = getpid();
				if (setpgid(0, 0) < 0) 	// PUT ALL foreground parallel into pid of 1st one
            	{
                	printf("Error: Setpgid\n");
                	_exit(1);
            	}
            }		
            else
            {
            	if (setpgid(0, lead) < 0) 	// PUT ALL foreground parallel into pid of 1st one
            	{
                	printf("Error: Setpgid\n");
                	_exit(1);
            	}
            }
*/
			execvp(tokens[0], tokens);
			// error in executing
			//https://stackoverflow.com/questions/53315081/how-child-processes-get-terminated-when-use-execve-loader-in-c
			printf("Error in executing %s (moving to next command)\n", tokens[0]);
			for(i = 0; tokens[i] != NULL; i++){
				free(tokens[i]);
			}
			free(tokens);
			_exit(1);		// reap dead children when fail to exec
			//exit(0);
		}
		else
		{	
			int stat;
			par_procs[par] = rc;
			if(par != num_parallel-1)
				continue;	// go on paralleling
			else
			{	// wait for all
				int reaped = 0;//number reaped
				while(reaped < num_parallel)
				{
					for(int parw = 0; parw < num_parallel; ++parw)
					{	
						if(par_procs[parw] == -1)	// already reaped
							continue;

						pid_t cpid = waitpid(par_procs[parw], &stat, WNOHANG); 	
						if (cpid == par_procs[parw]) 	//==0 means not exited
            			{	
            				par_procs[parw] = -1;
            				++reaped;
     						
        				}
					}
				}
				/*for(int parw = 0; parw < num_parallel; ++parw)
				{	
					int stat;
					pid_t cpid = waitpid(par_procs[parw], &stat, WNOHANG); 
        			if (cpid == background[i]) 	//==0 means not exited
            		{	
            			continue;
     				
        			}
        			else if(cpid == 0)	// add still in background
        			{
        			
        			}

					//waitpid(background[i], &stat, WNOHANG); 
					//waitpid(-abs(lead), NULL, 0);
					//wait(NULL);
				}*/
			}	
		}
		
	}	// for parallel for loop

	free(par_procs);

		for(i = 0; i < num_series; ++i)
		{	
			for(int j = 0; tokens_ser[i][j]!=NULL; ++j)
				free(tokens_ser[i][j]);
			free(tokens_ser[i]);
		}

		for(i = 0; i < num_parallel; ++i)
		{	
			for(int j = 0; tokens_par[i][j]!=NULL; ++j)
				free(tokens_par[i][j]);
			free(tokens_par[i]);

		}
		//setjmp(buf);
		free(tokens_ser);
		free(tokens_par);

		if(ctrl_C == 1)
			ctrl_C = 0;

	}
	//for(i = 0; i<num_series; i++){
	//		free(tokens_ser[i]);
	//	}
		
	return 0;
}

/*References:
https://www.geeksforgeeks.org/write-a-c-program-that-doesnt-terminate-when-ctrlc-is-pressed/
*/
