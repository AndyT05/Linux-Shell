// The MIT License (MIT)
//
// Copyright (c) 2024 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define WHITESPACE " \t\n"
#define MAX_COMMAND_SIZE 255
#define MAX_NUM_ARGUMENTS 32

int main(int argc, char *argv[])
{
  char error_message[30] = "An error has occurred\n";
  char *command_string = (char *)malloc(MAX_COMMAND_SIZE);

  FILE *input_file = NULL;

  if (argc > 2)
  {
    printf("too many argument");
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
  }

  if (argc == 2)
  {
    input_file = fopen(argv[1], "r");
    if (input_file == NULL)
    {
      printf("nothing in the file");
    }
  }

  while (1)
  {
    if (input_file == NULL)
    {
      // Interactive mode: print prompt
      printf("msh> ");
      // Read command from stdin
      if (!fgets(command_string, MAX_COMMAND_SIZE, stdin))
      {
        break;
      }
    }
    else
    {
      // Batch mode: read command from file
      if (!fgets(command_string, MAX_COMMAND_SIZE, input_file))
      {
        // End of file, exit
        break;
      }
    }

    // Tokenize input command
    char *token[MAX_NUM_ARGUMENTS];
    int token_count = 0;
    char *argument_pointer;
    char *working_string = strdup(command_string);
    char *head_ptr = working_string;

    while (((argument_pointer = strsep(&working_string, WHITESPACE)) != NULL) &&
           (token_count < MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup(argument_pointer, MAX_COMMAND_SIZE);
      if (strlen(token[token_count]) == 0)
      {
        token[token_count] = NULL;
      }
      token_count++;
    }

    // Execute built-in commands: exit and cd
    if (strcmp(token[0], "exit") == 0)
    {
      // Exit the shell
      exit(0);
    }
    else if (strcmp(token[0], "cd") == 0)
    {
      // Change directory
      if ((token_count - 1) != 2)
      {
        // Print error message for invalid arguments
        printf("\nInvalid argument");
      }
      else
      {
        // Change directory
        if (chdir(token[1]) == -1)
        {
          // Print error message for failed directory change
          printf("\nChange directory fail! ");
        }
      }
    }
    else
    {
      // Execute external commands
      pid_t pid = fork();
      if (pid == -1)
      {
        // Fork failed, print error message
        printf("\nFolk fail");
      }
      else if (pid == 0)
      {

        // Child process: execute command
        execvp(token[0], token);
        // If execv returns, an error occurred
        /**/
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(EXIT_FAILURE);
      }
      else
      {
        // Parent process: wait for child to terminate
        int status;
        waitpid(pid, &status, 0);
      }
    }

    // Free allocated memory
    free(head_ptr);
  }

  // Close input file if opened
  if (input_file != NULL)
  {
    fclose(input_file);
  }

  // Free allocated memory
  free(command_string);

  return 0;
}
