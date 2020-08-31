#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

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

	while(1) {			
		// check for background jobs
		int new_num_back = 0;

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
		num_back = new_num_back;

		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		if(argc == 2) { // batch mode
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	
			}
			line[strlen(line) - 1] = '\0';
		} else { // interactive mode
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}
		printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		if(strlen(line) == 0)
			continue;	// empty command move to next command// prompt again

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
				kill(background[i], SIGINT);
			}

			for(i = 0; tokens[i]!=NULL; i++){
				free(tokens[i]);
			}
			free(tokens);
			
			exit(0);
		}
		// if not cd then builtin commands with all diff modes

		int back = 0;	// not background
		for(i = 0; tokens[i] != NULL; i++);
		printf("%d", back);
		if(strcmp("&", tokens[i-1]) == 0)
		{
			back = 1; // it is a background process
			tokens[i-1] = NULL;	// remove & from it
			printf("%d", back);
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
		for(i = 0; i < MAX_NUM_TOKENS; ++i)
			tokens_ser[i] = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));

		char ***tokens_par = (char ***)malloc(MAX_NUM_TOKENS * sizeof(char **));
		for(i = 0; i < MAX_NUM_TOKENS; ++i)
			tokens_par[i] = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));


		for(i = 0; tokens[i] != NULL; ++i)
		{	
			//printf("%d", i);
			if(strcmp("&&", tokens[i]) == 0)
			{	
				//tokens_ser[num_series][j] = NULL;
				++num_series;
				j = 0;
				series = 1;
				continue;
			}
			tokens_ser[num_series][j] = tokens[i];
			++j;
		}
		// to read last one 

		// check for ||
		int num_parallel = 0;
		j = 0;
		// look for parallel
		for(i = 0; tokens[i] != NULL; ++i)
		{	
			if(strcmp("&&&", tokens[i]) == 0)
			{
				++num_parallel;
				j = 0;
				parallel = 1;
				continue;
			}
			tokens_par[num_parallel][j] = tokens[i];
			++j;
		}
		
		//printf("%d", series);
		if(parallel==0 && series==0)	// only 1 command
		{	
			num_series = 1;
			series = 1;
		}
		else if(parallel == 0)	// last command not counted in num_series
			num_series++;
		else if(series == 0)	// last command not counted in num_parallel
			num_parallel++;
		/*for(i = 0; tokens[i] != NULL; i++){
			free(tokens[i]);
		}
		free(tokens);*/
		//printf("%d", series);
		// series may have series or 1 background
		printf("\n%d\n", num_series);
		for(int ser = 0; ser < num_series; ++ser){
			tokens = tokens_ser[ser];			
		int rc = fork();
		if(rc < 0)	// failed to fork
		{
			printf("Failed to fork: moving to next command\n");
		}
		
		else if(rc == 0) // child
		{
			execvp(tokens[0], tokens);
				// error in executing
				//https://stackoverflow.com/questions/53315081/how-child-processes-get-terminated-when-use-execve-loader-in-c
			printf("Error in executing %s (moving to next command)\n", tokens[0]);
			//for(i = 0; tokens[i] != NULL; i++){
			//	free(tokens[i]);
			//}
			//free(tokens);
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
		/*for(i = 0; tokens[i] != NULL; i++){
			free(tokens[i]);
		}*/
		//free(tokens);
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
		for(int par = 0; par < num_parallel; ++par){
			tokens = tokens_par[par];			
		int rc = fork();
		if(rc < 0)	// failed to fork
		{
			printf("Failed to fork: moving to next command\n");
		}
		
		else if(rc == 0) // child
		{
			execvp(tokens[0], tokens);
			// error in executing
			//https://stackoverflow.com/questions/53315081/how-child-processes-get-terminated-when-use-execve-loader-in-c
			printf("Error in executing %s (moving to next command)\n", tokens[0]);
			//for(i = 0; tokens[i] != NULL; i++){
			//	free(tokens[i]);
			//}
			//free(tokens);
			_exit(1);		// reap dead children when fail to exec
			//exit(0);
		}
		else
		{	
			if(par != num_parallel-1)
				continue;	// go on paralleling
			else
			{	// wait for all
				for(int parw = 0; parw < num_parallel; ++parw)
				{	wait(NULL);
				}
			}	
		}
		
	}	// for parallel for loop
		/*for(i = 0; i < MAX_NUM_TOKENS; ++i)
		{	
			free(tokens_ser[i]);
			free(tokens_par[i]);

		}*/
		free(tokens_ser);
		free(tokens_par);


	}
	//for(i = 0; i<num_series; i++){
	//		free(tokens_ser[i]);
	//	}
		
	return 0;
}
