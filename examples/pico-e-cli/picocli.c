#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "picocli.h"

// commands
#include "commands/testcommand.h"
#include "commands/systemcommand.h"

const struct WX_COMMAND g_WXCmdTable[] =
{
    { "system",  system_command,  "system_command...." },
    { "w5x00",   w5x00_command,  "w5x00_command...." },
};

void run_command(const char *input, int argc, char *argv[])
{
	for(int i = 0; i < sizeof(g_WXCmdTable) / sizeof(g_WXCmdTable[0]); i++)
	{
		int len = strlen(g_WXCmdTable[i].cmd);

		if ( strcmp(g_WXCmdTable[i].cmd, input)==0 )
		{
			g_WXCmdTable[i].process(argc, argv);
			break;
		}
	}
}

char* simple_fgets(char* str, int num, FILE* stream)
{
    int i = 0;
    int c;

    while(1)
    {
        int getchar_timeout_us(uint32_t timeout_us);
        c = getchar_timeout_us(1);
        if ( c<0 )
        {
            //printf(".");
            void poll_w5x00macraw(int check_linkup);
            poll_w5x00macraw(1);
            continue;
        }
        if ( !(i < num - 1 && c != EOF && c != '\n' && c != '\r') ) break;

        str[i++] = c;
        putchar(c);
    }

    if ( (c == '\n') || (c == '\r') )
    {
        str[i++] = c;
        putchar('\r');
        putchar('\n');
    }
    str[i] = '\0';
    return (i == 0 && c == EOF) ? NULL : str;
}

void picocli_startup(int logo)
{
    if (logo==1)
    {
        printf("#################### Pico Terminal ####################\n");
    }
    printf(">> ");
}

void picocli_loop()
{
    char input[MAX_STRING_SIZE];

    while (simple_fgets(input, sizeof(input), stdin))
    {
        input[strlen(input) - 1] = '\0';

        if ( strcmp(input, "exit")==0 )  break;

        char *tokens = strtok(input, " ");

        char *argv[MAX_STRING_SIZE] = {0,};
        int argc = 0;
        while (tokens != NULL)
        {
            argv[argc] = strdup(tokens);
            tokens = strtok(NULL, " ");

            argc++;
        }

        run_command(input, argc, argv);

        picocli_startup(0);
    }
}


