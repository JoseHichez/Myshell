//Jose Hichez
//Panther ID: 6186740

/*I affirm that I wrote this code by myself without help from
 students or sources online.*/

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ARGS 20
#define BUFSIZE 1024
// come from myshell.c in the example.

int get_args(char* cmdline, char* args[])
{
  int i = 0;

  /* if no args */
  if((args[0] = strtok(cmdline, "\n\t ")) == NULL)
    return 0; //returns 0

  while((args[++i] = strtok(NULL, "\n\t ")) != NULL) { //while there are args
    if(i >= MAX_ARGS) {
      printf("Too many arguments!\n");  //print if max args is reached
      exit(1);
    }
  }
  /* the last one is always NULL */
  return i;
}
/*
	Parses the file interaction part of the command.
	Which are ">", "<", and ">>", when the algorithm found the symbol,
	the algorithm will set flag and store the specified file name.
	And then pipe the contents.
*/
void execute(char* singleCommand)
{
	int pid, async;
	char* args[MAX_ARGS];

	int nargs = get_args(singleCommand, args);
	if (nargs <= 0) return;

	/* check if async call */
	if(!strcmp(args[nargs-1], "&")) { async = 1; args[--nargs] = 0; }
	else async = 0;

	int wFlag = 0, aFlag = 0, rFlag = 0;
	char* outfile;
	char* infile;
	int i = 0;
	for(i = 0 ; i < nargs; i++)  //starts the parsing
	{
		if(strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0)   //checks for ">" and >>
		{
			//rewrite output to the specific file.
			if (strcmp(args[i], ">") == 0)
				wFlag = 1;
			//append output to the specific file.
			else if (strcmp(args[i], ">>") == 0) //checks for ">>"
				aFlag = 1;
			args[i] = 0;
			i++;
			outfile = args[i];
			args[i] = 0;
		}
		//getting input from the specific file.
		else if (strcmp(args[i], "<") == 0){ //check "<"
			rFlag = 1;
			args[i] = 0;
			i++;
			infile = args[i];
			args[i] = 0;
		}
	}
	if(wFlag == 1 && aFlag == 1)  //checks flags
	{
		printf("Error: >> and > cannot be use at the same time.\n");
		exit(1);
	}
	FILE* in;
	FILE* out;
	if (rFlag == 1){
		in = fopen(infile, "r");
		if (in == NULL)
		{
			printf("Error: cannot open input file.\n");
			exit(1);
		}
		dup2(fileno(in), STDIN_FILENO);
	}
	if(wFlag == 1 || aFlag == 1)
	{
		if (wFlag == 1)
			out = fopen(outfile, "w");
		else if (aFlag == 1)
			out = fopen(outfile, "a");
		if (out == NULL)
		{
			printf("Error: cannot open output file.\n");
			exit(1);
		}
		dup2(fileno(out), STDOUT_FILENO);
	}
	execvp(args[0], args); // execute the command.
	perror("exec failed");
	exit(-1);
}
/*
	Forks those commands into different child processes.
	Separates the arguments by pipe symbol "|". When more than one pipe appeared,
	the algorithm will turns into a for loop for the ends between but not include
	the first one and the last one. If there has only 2 ends for the pipe,
	the loop will be skipped.
*/
int main (int argc, char* argv [])
{
	char cmdline[BUFSIZE];
	for(;;) {
		printf("COP4338$ ");
		if(fgets(cmdline, BUFSIZE, stdin) == NULL) {
		  perror("fgets failed");
		  exit(1);
		}
		if(!strcmp(cmdline, "quit\n") || !strcmp(cmdline, "exit\n")) {
			return 0;
		}
		int pid;
		char* childCommands[MAX_ARGS];
		int count = 0;
		if ((childCommands[0] = strtok(cmdline, "|")) == NULL){
			count = 1;
		}
		while ((childCommands[++count] = strtok(NULL, "|")) != NULL);

		pid = fork();
		if (pid == 0){

			if (count == 1)
			{
				execute(childCommands[0]);
			}
			else if(count > 1)
			{
				int fpid, status;
				int fd[count - 1][2];
				// first end.
				pipe(fd[0]);
				fpid = fork();
				if (fpid == 0){
					dup2(fd[0][1], 1);
					close(fd[0][0]);
					execute(childCommands[0]);
				}
				//Loop for the ends of pipes between but not include the first one and the last one.
				//If there has only 2 ends, the loop will be skipped.
				int i = 1;
				for(i = 1; i < count - 1; i++)
				{
					pipe(fd[i]);
					fpid = fork();
					if (fpid == 0){
						dup2(fd[i - 1][0], 0);
						dup2(fd[i][1], 1);
						close(fd[i - 1][1]);
						close(fd[i][0]);
						execute(childCommands[i]);
					}
					close(fd[i - 1][0]);
					close(fd[i - 1][1]);
				}
				// last end.
				fpid = fork();
				if (fpid == 0){
					dup2(fd[count - 2][0], 0);
					close(fd[count - 2][1]);
					execute(childCommands[count - 1]);
				}
				close(fd[count - 2][0]);
				close(fd[count - 2][1]);
				while ((fpid = wait(&status)) != -1);
			}
		}
		else if (pid > 0){
			waitpid(pid, NULL, 0);
		}
		else {
			perror("fork failed");
			exit(1);
		}
	}
	return 0;
}
