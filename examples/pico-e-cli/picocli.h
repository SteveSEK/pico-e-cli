#pragma once

#define MAX_STRING_SIZE 500

struct WX_COMMAND
{
	const char *cmd;
	void (*process)(int argc, char *argv[]);
	char *description;
};

void picocli_startup(int logo);
void picocli_loop();
