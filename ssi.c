// Shidhi Mohammad Bin Arif
// V00911512

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

//bg_process singly linked list that stores background process info
struct bg_process
{
    int pid;
    char **argv;
    struct bg_process *next;
};

struct bg_process *head = NULL;
struct bg_process *curr = NULL;

// linked list insert method
struct bg_process *insertNode(int pid, char **argv)
{

    struct bg_process *newNode = (struct bg_process *)malloc(sizeof(struct bg_process));
    newNode->pid = pid;
    newNode->argv = argv;
    newNode->next = NULL;
    curr->next = newNode;
    curr = newNode;

    return newNode;
}

// linked list delete method
int deleteNode(int pid)
{
    struct bg_process *tmp = NULL;
    struct bg_process *prev = NULL;

    if (tmp != NULL && tmp->pid == pid)
    {
        prev = tmp->next;
        free(tmp);
        tmp = NULL;
        return 0;
    }

    else
    {
        while (tmp != NULL && tmp->pid != pid)
        {
            prev = tmp;
            tmp = tmp->next;
        }

        if (tmp == NULL)
            return 0;

        prev->next = tmp->next;

        free(tmp);
        tmp = NULL;
    }
}

// backgroundProcessCheck is a global variable to check if any bg processes are running
int backgroundProcessCheck = 0;

//  lists currently running background processes
int bglist(char **argv)
{
    char cwd[PATH_MAX];
    int count = 0;
    struct bg_process *pointer = head->next;

    while (pointer != NULL)
    {
        getcwd(cwd, sizeof(cwd));
        printf("%d: %s/ %d\n", pointer->pid, cwd, count);
        count++;
        pointer = pointer->next;
    }
    printf("Total Background Jobs: %d\n", count);
}

// change directory method
int cd(char **argv)
{
    char *home = getenv("HOME");
    if (argv[1] == NULL || strcmp(argv[1], "~") == 0)
    {
        chdir(home);
    }
    else
    {
        if (chdir(argv[1]) != 0)
        {
            perror("SSI");
        }
    }
    return 1;
}

// exit from shell method
int terminate(char **argv)
{
    return 0;
}

// start 
int startProcess(char **argv)
{
    pid_t pid;
    int status;

    if (backgroundProcessCheck == 1)
    {
        pid = fork();
        if (pid == 0)
        {
            execvp(argv[0], argv);
            printf("errno is %d\n", errno);
        }
        else
        {
            insertNode(pid, argv);
        }
        backgroundProcessCheck = 0;
    }
    else
    {
        pid = fork();
        if (pid == 0)
        {
            // Child processes
            if (execvp(argv[0], argv) == -1)
            {
                perror("SSI");
            }
            exit(1);
        }
        else if (pid < 0)
        {
            // Error forking
            perror("SSI");
        }
        else
        {
            if (!WIFEXITED(status) && !WIFSIGNALED(status))
            {
                wait(NULL);
            }
        }
    }
    while (1)
    {
        pid_t ter = waitpid(0, &status, WNOHANG);
        if (ter <= 0)
        {
            break;
        }
        deleteNode(pid);
        printf("\nProcess %d has terminated with code %d\n", ter, WEXITSTATUS(status));
    }
    return 1;
}

// executes commands called in the interpreter
int execute(char **argv)
{
    if (argv[0] == NULL)
    {
        return 1;
    }

    if (strcmp(argv[0], "cd") == 0)
    {
        return cd(argv);
    }
    else if (strcmp(argv[0], "exit") == 0)
    {
        return terminate(argv);
    }
    else if (strcmp(argv[0], "bglist") == 0)
    {
        return bglist(argv);
    }
    return startProcess(argv);
}

// reads the line from the interpreter
char *getLine(void)
{
    char *CLI_input_line = NULL;
    ssize_t buffer = 0;

    if (getline(&CLI_input_line, &buffer, stdin) == -1)
    {
        if (feof(stdin))
        {
            exit(0);
        }
        else
        {
            perror("readline");
            exit(1);
        }
    }

    return CLI_input_line;
}

// tokenizes and separates the commands and arguments
char **tokenize(char *CLI_input_line)
{
    int place = 0;
    char **argv = malloc(128 * sizeof(char *));
    char *tempToken;

    tempToken = strtok(CLI_input_line, " \n");
    while (tempToken != NULL)
    {
        if (strcmp(tempToken, "bg") == 0)
        {
            backgroundProcessCheck = 1;
        }
        else
        {
            argv[place] = tempToken;
            place++;
        }

        tempToken = strtok(NULL, " \n");
    }
    argv[place] = NULL;
    return argv;
}

// initializes list
struct bg_process *initialize()
{
    head = (struct bg_process *)malloc(sizeof(struct bg_process));
    head->next = NULL;
    head->pid = 0;
    struct bg_process *p = (struct bg_process *)(malloc(sizeof(struct bg_process *)));
    p->pid = 0;
    p->next = NULL;
    head = curr = p;
}

int main()
{
    initialize();

    char cwd[PATH_MAX];
    char host_name[HOST_NAME_MAX];
    char *user = getenv("USER");
    gethostname(host_name, HOST_NAME_MAX);
    char *CLI_input_line;
    char **argv;
    int status;

    do
    {
        getcwd(cwd, sizeof(cwd));
        printf("%s@%s: %s > ", user, host_name, cwd);
        CLI_input_line = getLine();
        argv = tokenize(CLI_input_line);
        status = execute(argv);
        free(argv);
    } while (status);

    return 0;
}
