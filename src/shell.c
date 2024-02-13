#include <shell.h>

/*
  Function Declarations for builtin shell commands:
 */
int psh_cd(char **args);
int psh_help(char **args);
int psh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
    "cd",
    "help",
    "pwd",
    "exit"};

int (*builtin_func[])(char **) = {
    &psh_cd,
    &psh_help,
    &psh_pwd,
    &psh_exit};

char *current_dir = NULL; /* Current working directory */

int psh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

/**
   @brief Builtin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int psh_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "psh: expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("psh");
        }
    }
    return 1;
}

int psh_pwd(char **args)
{
    if (current_dir == NULL)
    {
        current_dir = malloc(1024 * sizeof(char));
    }
    getcwd(current_dir, 1024);
    printf("%s\n", current_dir);
    return 1;
}

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int psh_help(char **args)
{
    int i;
    printf("The following are built in:\n");

    for (i = 0; i < psh_num_builtins(); i++)
    {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int psh_exit(char **args)
{
    return 0;
}

char *concat_str(size_t nmemb, ...)
{
    int size_str = 0;
    va_list ap;
    va_start(ap, nmemb);

    char *concat_str = malloc(sizeof(char));
    concat_str[0] = '\0';

    for (int i = 0; i < nmemb; i++)
    {
        char *sstr = va_arg(ap, char *);
        if (!sstr)
        {
            break;
        }

        size_str += strlen(sstr);
        concat_str = realloc(concat_str, size_str + 1 * sizeof(char));
        if (concat_str == NULL)
        {
            printf("Error realloc\n");
            exit(EXIT_FAILURE);
        }
        strncat(concat_str, sstr, strlen(sstr));
    }
    va_end(ap);
    return concat_str;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int psh_launch(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        // Concat path to args[0]
        char *function_file = concat_str(3, PATH, "/", args[0]);
        // Child process
        if (execvp(function_file, args) == -1)
        {
            perror("psh");
        }
        free(function_file);
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        // Error forking
        perror("psh");
    }
    else
    {
        // Parent process
        do
        {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int psh_execute(char **args)
{
    int i;

    if (args[0] == NULL)
    {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < psh_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }

    return psh_launch(args);
}

/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *psh_read_line(void)
{
#ifdef psh_USE_STD_GETLINE
    char *line = NULL;
    ssize_t bufsize = 0; // have getline allocate a buffer for us
    if (getline(&line, &bufsize, stdin) == -1)
    {
        if (feof(stdin))
        {
            exit(EXIT_SUCCESS); // We received an EOF
        }
        else
        {
            perror("psh: getline\n");
            exit(EXIT_FAILURE);
        }
    }
    return line;
#else
#define psh_RL_BUFSIZE 1024
    int bufsize = psh_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer)
    {
        fprintf(stderr, "psh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Read a character
        c = getchar();

        if (c == EOF)
        {
            exit(EXIT_SUCCESS);
        }
        else if (c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate.
        if (position >= bufsize)
        {
            bufsize += psh_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer)
            {
                fprintf(stderr, "psh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
#endif
}

#define psh_TOK_BUFSIZE 64
#define psh_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **psh_split_line(char *line)
{
    int bufsize = psh_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token, **tokens_backup;

    if (!tokens)
    {
        fprintf(stderr, "psh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, psh_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += psh_TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                free(tokens_backup);
                fprintf(stderr, "psh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, psh_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void psh_loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        printf("> ");
        line = psh_read_line();
        args = psh_split_line(line);
        status = psh_execute(args);

        free(line);
        free(args);
    } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
    // Load config files, if any.

    // Run command loop.
    psh_loop();

    // Perform any shutdown/cleanup.

    return EXIT_SUCCESS;
}