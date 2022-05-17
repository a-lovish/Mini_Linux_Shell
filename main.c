#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

char* read_input();
int no_pipe(char *);
void tok_pipe1(char*, char**);
void tok_pipe2(char*, char**);
char** tok_input(char*);
int run(char**);
int run_EV(char**);
int run_cd(char**);
int run_history(char**);
int run_echo(char**);
int set_manualEV(char **);
int help(char **);
int run_ext(char**);
int run_pipe1(char**, char**);
int run_pipe2(char**, char**, char**);
int check_rediIn(char **,char **);
int check_rediOut(char **,char **);
int check_rediApp(char**,char**);

FILE *fp;

int main()
{
    printf("\033[H\033[J");

    char *line;
    char **args;
    int status;
    char s[100];
    int pipe = 0;
    do
    {
        printf("%s ",getcwd(s,100));
        line = read_input();
        pipe = no_pipe(line);
        if(pipe==0)
        {

            args = tok_input(line);
            status = run(args);
        }
        if(pipe==1)
        {

            char* pipeArg[2];
            char** pipe1;
            char** pipe2;
            tok_pipe1(line,pipeArg);
            pipe1 = tok_input(pipeArg[0]);
            pipe2 = tok_input(pipeArg[1]);
            if(pipe1[0] == NULL || pipe2[0] == NULL)
            {
                status = 1;
                printf("Give proper command.\n");
            }
            else
            {
                status = run_pipe1(pipe1, pipe2);
            }
        }
        if(pipe==2)
        {
            char* pipeArg[3];
            char** pipe1;
            char** pipe2;
            char** pipe3;
            tok_pipe2(line,pipeArg);
            pipe1 = tok_input(pipeArg[0]);
            pipe2 = tok_input(pipeArg[1]);
            pipe3 = tok_input(pipeArg[2]);
            if(pipe1[0] == NULL || pipe2[0] == NULL || pipe3[0] == NULL)
            {
                status = 1;
                printf("Give proper command.\n");
            }
            else
            {
                status = run_pipe2(pipe1, pipe2,pipe3);
            }
        }
    }
    while (status);
    return 0;
}

//Read input from user
char* read_input(void)
{
    int size = 512;
    int p = 0;
    char *in = (char *)malloc(sizeof(char)*size);
    int c;
    fp=fopen("/tmp/history.txt","a");
    c=getchar();
    if(c=='\n')
    {
        fclose(fp);
        in[p] = '\0';
        return in;
    }
    for(;;)
    {
        if (c != EOF && c != '\n')
        {
            in[p] = c;
            fputc(c,fp);
        }
        else
        {
            fputc('\n',fp);
            fclose(fp);
            in[p] = '\0';
            return in;
        }
        p++;
        //When input size exceed size of in
        if (p >= size)
        {
            size += 512;
            in = (char *)realloc(in, size);
        }
        c=getchar();
    }
}

//Count number of pipes.
int no_pipe(char* line)
{
    int i=0,a=0;
    while(line[i]!= '\0')
    {
        if(line[i]=='|')
        {
            a++;
        }
        i++;
    }
    return a++;
}

//Parse pipe when number of pipe is 1.
void tok_pipe1(char* line, char** pipeArgs)
{
    pipeArgs[0]=strtok(line, "|");
    pipeArgs[1]=strtok(NULL, "|");
}

//Parse pipe when number of pipe is 2.
void tok_pipe2(char* line, char** pipeArgs)
{
    pipeArgs[0]=strtok(line, "|");
    pipeArgs[1]=strtok(NULL, "|");
    pipeArgs[2]=strtok(NULL, "|");
}

//Parse spaces in input.
char** tok_input(char *line)
{
    int size = 64;
    int p = 0;
    char **tokens = (char **)malloc(size * sizeof(char*));
    char *token;
    token = strtok(line, " \t\n");
    while (token != NULL)
    {
        tokens[p] = token;
        p++;
        if (p >= size)
        {
            size += 64;
            tokens = (char **)realloc(tokens, size * sizeof(char*));
        }
        token = strtok(NULL, " \t\n");
    }
    tokens[p] = NULL;
    return tokens;
}

//Check for input redirection.
int check_rediIn(char** args,char** input)
{
    int i=0;
    int j=0;
    do
    {
        if(args[i][0] == '<')
        {
            *input = strtok(args[i],"<");
            if(*input!=NULL)
            {
                for(j = i; args[j] != NULL; j++)
                {
                    args[j] = args[j+1];
                }
                return 1;
            }
            else
            {
                if(args[i+1]!=NULL)
                {
                    *input = args[i+1];
                    for(j = i; args[j-1] != NULL; j++)
                    {
                        args[j] = args[j+2];
                    }
                    return 1;
                }
                return 0;
            }
        }
        i++;
    }
    while(args[i] != NULL);
    return -1;
}

//Check for output redirection.
int check_rediOut(char** args,char** output)
{
    int i=0;
    int j;
    do
    {
        if(args[i][0] == '>' && args[i][1] != '>')
        {
            *output = strtok(args[i],">");
            if(*output!=NULL)
            {
                for(j = i; args[j] != NULL; j++)
                {
                    args[j] = args[j+1];
                }
                return 1;
            }
            else
            {
                if(args[i+1]!=NULL)
                {
                    *output = args[i+1];
                    for(j = i; args[j-1] != NULL; j++)
                    {
                        args[j] = args[j+2];
                    }
                    return 1;
                }
                return 0;
            }
        }
        i++;
    }
    while(args[i] != NULL);
    return -1;
}

//Check for append redirection.
int check_rediApp(char** args,char** append)
{
    int i=0;
    int j;
    do
    {
        if(args[i][0] == '>' && args[i][1] == '>')
        {
            *append = strtok(args[i],">");
            if(*append!=NULL)
            {
                for(j = i; args[j] != NULL; j++)
                {
                    args[j] = args[j+1];
                }
                return 1;
            }
            else
            {
                if(args[i+1]!=NULL)
                {
                    *append = args[i+1];
                    for(j = i; args[j-1] != NULL; j++)
                    {
                        args[j] = args[j+2];
                    }
                    return 1;
                }
                return 0;
            }
        }
        i++;
    }
    while(args[i] != NULL);
    return -1;
}

//Run some internal commands.
int run(char **args)
{
    if(args[0] == NULL)
    {
        return 1;
    }

    if(args[0][0] == '<' || args[0][0] == '>')
    {
        printf("Enter some command.\n");
        return 1;
    }

    if(strcmp(args[0],"echo")==0)
    {
        return (run_echo(args));
    }


    if(strcmp(args[0],"setenv")==0)
    {
        return (set_manualEV(args));
    }


    if(strcmp(args[0],"cd")==0)
    {
        return (run_cd(args));
    }

    if(strcmp(args[0],"exit")==0 || strcmp(args[0],"quit")==0 || strcmp(args[0],"x")==0)
    {
         remove("/tmp/history.txt");
         return (0);
    }

    if(strcmp(args[0],"help")==0)
    {
        return (help(args));
    }

    return (run_ext(args));
}

//Prints environment variable.
int run_EV(char** args)
{
    char* EV=getenv(args[1]);
    if(EV==NULL)
    {
        printf("\n");
        return 1;
    }
    printf("%s\n",EV);
    return 1;
}

//Change directory of program.
int run_cd(char** args)
{
    if(args[1]==NULL)
    {
        chdir(getenv("HOME"));
        return 1;
    }
    if (chdir(args[1]) != 0)
    {
      perror("bash");
    }
    return 1;
}

//Prints history.
int run_history(char** args)
{
    fp=fopen("/tmp/history.txt","r");
    int c;
    while((c=fgetc(fp))!=EOF)
    {
        putchar(c);
    }
    fclose(fp);
    return 1;
}

//Runs echo.
int run_echo(char** args)
{
    if(args[1]==NULL)
    {
        printf("\n");
        return 1;
    }
    if(args[1][0] != '$')
    {
        return (run_ext(args));
    }
    else
    {
        args[1] = strtok(args[1],"$");
        if(args[1] == NULL)
        {
            printf("\n");
        }
        else
        {
            char* EV=getenv(args[1]);
            if(EV==NULL)
            {
                printf("\n");
                return 1;
            }
            printf("%s\n",EV);
        }
    }
    return 1;
}

//Sets environment variable.
int set_manualEV(char** args)
{
    if(args[2] != NULL )
    {
        if(strcmp(args[1],"HOME")!=0)
        {
            setenv(args[1],args[3],1);
        }
        return 1;
    }
    else
    {
        char* name;
        char* value;
        name=strtok(args[1],"=");
        value=strtok(NULL,"=");
        if(strcmp(args[1],"HOME")!=0)
        {
            setenv(name, value, 1);
        }
        return 1;
    }
}

//Prints help command.
int help(char** args)
{
    printf("\nWELCOME TO MY SHELL\nNot all the internal commands are supported.\nSupported internal commands: cd, pwd, echo, history, exit.\n");
    printf("Commands can be piped together(Upto two pipes).\n");
    printf("Commands with redirection can be piped together(Only for single pipes).\n");
    printf("Input redirection from file is supported: eg. wc -c < fileInput\nOutput redirection to file is supported: eg. ls > fileOutput\n");
	printf("Output redirection to file with append is supported: eg. ls >> fileOutput\n");
	printf("Both Input redirection from file and Output redirection to file in single command is supported: eg. wc -c < fileInput > fileOutput\n");
	printf("For more detail please read README file.\n\n");
    return 1;
}

//Runs external command and some internal command with redirection.
int run_ext(char **args)
{
    char *input;
    char *output;
    char *append;
    int status,r;
    int in=check_rediIn(args,&input);
    int out=check_rediOut(args,&output);
    int app=check_rediApp(args,&append);
    int fd1,fd2;
    if(in==0)
    {
        printf("Enter command again with input file name.\n");
        return 1;
    }
    if(out==0)
    {
        printf("Enter command again with output file name.\n");
        return 1;
    }
    if(app==0)
    {
        printf("Enter command again with name of file to be appended.\n");
        return 1;
    }
    if(in==1)
    {
        fd1=open(input,O_RDONLY);
        if(fd1<0)
        {
            fprintf(stderr,"ERROR opening the input file.\n");
            return 1;
        }
    }
    if(out==1)
    {
        fd2=open(output,O_WRONLY | O_CREAT, 0777 );
        if(fd2<0)
        {
            fprintf(stderr,"ERROR outputting the file.\n");
            return 1;
        }
    }
    if(app==1)
    {
        fd2=open(append,O_WRONLY | O_APPEND | O_CREAT, 0777);
        if(fd2<0)
        {
            fprintf(stderr,"ERROR appending the file.\n");
            return 1;
        }
    }
    pid_t i;
    i = fork();
    if (i < 0)
    {
        fprintf(stderr,"ERROR forking the file.\n");
    }
    if (i==0)
    {
        if(in==1)
        {
            dup2(fd1,STDIN_FILENO);
        }
        if(out==1)
        {
            dup2(fd2, STDOUT_FILENO);
        }
        if(app==1)
        {
            dup2(fd2, STDOUT_FILENO);
        }
        int a=0;
        if(strcmp(args[0],"printenv")==0)
        {
            a=run_EV(args);
        }
        if(strcmp(args[0],"history")==0)
        {
            a=run_history(args);
        }
        if(a==0)
        {
            if(execvp(args[0], args) < 0)
            {
                fprintf(stderr, "No such command found.\n");
                exit(EXIT_FAILURE);
            }
        }
        exit(EXIT_SUCCESS);
    }

    if(in==1)
    {
        close(fd1);
    }
    if(out==1)
    {
        close(fd2);
    }
    if(app==1)
    {
        close(fd2);
    }
    r = waitpid(i, &status, 0);
    return 1;
}

//Function to work when one pipe is given.
int run_pipe1(char** pipe1, char** pipe2)
{
    int piping[2], r, status;
    char *input;
    char *output;
    char *append;
    int in=check_rediIn(pipe1,&input);
    int out=check_rediOut(pipe2,&output);
    int app=check_rediApp(pipe2,&append);
    int fd1,fd2;
    if(in==0)
    {
        printf("Enter command again with input file name.\n");
        return 1;
    }
    if(out==0)
    {
        printf("Enter command again with output file name.\n");
        return 1;
    }
    if(app==0)
    {
        printf("Enter command again with name of file to be appended.\n");
        return 1;
    }
    if(in==1)
    {
        fd1=open(input,O_RDONLY);
        if(fd1<0)
        {
            fprintf(stderr,"ERROR opening the input file.\n");
            return 1;
        }
    }
    if(out==1)
    {
        fd2=open(output, O_WRONLY | O_CREAT, 0777 );
        if(fd2<0)
        {
            fprintf(stderr,"ERROR outputting the file.\n");
            return 1;
        }
    }
    if(app==1)
    {
        fd2=open(append,O_WRONLY | O_APPEND | O_CREAT, 0777);
        if(fd2<0)
        {
            fprintf(stderr,"ERROR appending the file.\n");
            return 1;
        }
    }

    pipe(piping);
    pid_t c1 = fork();

    if(c1 < 0)
    {
        fprintf(stderr,"ERROR forking the file.\n");
    }
    if (c1 == 0)
    {
        if(in==1)
        {
            dup2(fd1,STDIN_FILENO);
        }
        dup2(piping[1], STDOUT_FILENO);
        close(piping[0]);
        close(piping[1]);
        int a=0;
        if(strcmp(pipe1[0],"printenv")==0)
        {
            a=run_EV(pipe1);
        }
        if(strcmp(pipe1[0],"history")==0)
        {
            a=run_history(pipe1);
        }
        if(a==0)
        {
            if(execvp(pipe1[0], pipe1) < 0)
            {
                fprintf(stderr, "No such command found.\n");
                exit(EXIT_FAILURE);
            }
        }
        exit(EXIT_SUCCESS);
    }
    assert(c1 > 0);

    pid_t c2 = fork();
    if(c1 < 0)
    {
        fprintf(stderr,"ERROR forking the file.\n");
    }
    if (c2 == 0)
    {
        dup2(piping[0], STDIN_FILENO);
        close(piping[0]);
        close(piping[1]);
        if(out==1)
        {
            dup2(fd2, STDOUT_FILENO);
        }
        if(app==1)
        {
            dup2(fd2, STDOUT_FILENO);
        }
        int a=0;
        if(strcmp(pipe2[0],"printenv")==0)
        {
            a=run_EV(pipe2);
        }
        if(strcmp(pipe2[0],"history")==0)
        {
            a=run_history(pipe2);
        }
        if(a==0)
        {
            if(execvp(pipe2[0], pipe2) < 0)
            {
                fprintf(stderr, "No such command found.\n");
                exit(EXIT_FAILURE);
            }
        }
        exit(EXIT_SUCCESS);
    }
    assert(c2 > 0);

	close(piping[0]);
	close(piping[1]);
	if(in==1)
    {
        close(fd1);
    }
    if(out==1)
    {
        close(fd2);
    }
    if(app==1)
    {
        close(fd2);
    }
	r=waitpid(c1, &status, 0);
	r=waitpid(c2, &status, 0);
    return 1;
}

//Function to work when two pipes are given.
int run_pipe2(char** arg1, char** arg2, char** arg3)
{
    int r,status;
    int piping[4];

    pipe(piping);
    pipe(piping + 2);

    pid_t c1=fork();
    if(c1 < 0)
    {
        fprintf(stderr,"ERROR forking the file.\n");
    }
    if(c1 == 0)
    {
        dup2(piping[1], STDOUT_FILENO);
        close(piping[0]);
        close(piping[1]);
        close(piping[2]);
        close(piping[3]);
        int a=0;
        if(strcmp(arg1[0],"printenv")==0)
        {
            a=run_EV(arg1);
        }
        if(strcmp(arg1[0],"history")==0)
        {
            a=run_history(arg1);
        }
        if(a==0)
        {
            if(execvp(arg1[0], arg1)==-1)
            {
                fprintf(stderr, "No such command found.\n");
                exit(EXIT_FAILURE);
            }
        }
        exit(EXIT_SUCCESS);
    }
    assert(c1 > 0);

    pid_t c2 = fork();
    if(c2 < 0)
    {
        fprintf(stderr,"ERROR forking the file.\n");
    }
    if(c2 == 0)
	{
	    dup2(piping[0], STDIN_FILENO);
	    dup2(piping[3], STDOUT_FILENO);
        close(piping[0]);
        close(piping[1]);
        close(piping[2]);
        close(piping[3]);
        int a=0;
        if(strcmp(arg2[0],"printenv")==0)
        {
            a=run_EV(arg2);
        }
        if(strcmp(arg2[0],"history")==0)
        {
            a=run_history(arg2);
        }
        if(a==0)
        {
            if(execvp(arg2[0], arg2)==-1)
            {
                fprintf(stderr, "No such command found.\n");
                exit(EXIT_FAILURE);
            }
        }
        exit(EXIT_SUCCESS);
	}
	assert(c2 > 0);

    pid_t c3 = fork();
    if(c3 < 0)
    {
        fprintf(stderr,"ERROR forking the file.\n");
    }
    if(c3 == 0)
    {
        dup2(piping[2], STDIN_FILENO);
        close(piping[0]);
        close(piping[1]);
        close(piping[2]);
        close(piping[3]);
        int a=0;
        if(strcmp(arg3[0],"printenv")==0)
        {
            a=run_EV(arg3);
        }
        if(strcmp(arg3[0],"history")==0)
        {
            a=run_history(arg3);
        }
        if(a==0)
        {
            if(execvp(arg3[0], arg3)==-1)
            {
                fprintf(stderr, "No such command found.\n");
                exit(EXIT_FAILURE);
            }
        }
        exit(EXIT_SUCCESS);
    }
    close(piping[0]);
    close(piping[1]);
    close(piping[2]);
    close(piping[3]);
    r = waitpid(c1, &status, 0);
	r = waitpid(c2, &status, 0);
	r = waitpid(c3, &status, 0);
    return 1;
}
