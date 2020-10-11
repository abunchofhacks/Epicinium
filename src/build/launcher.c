#ifdef PLATFORMDEBIAN32
#define PLATFORMUNIX
#endif
#ifdef PLATFORMDEBIAN64
#define PLATFORMUNIX
#endif
#ifdef PLATFORMOSX32
#define PLATFORMUNIX
#define PLATFORMOSX
#endif
#ifdef PLATFORMOSX64
#define PLATFORMUNIX
#define PLATFORMOSX
#endif

#ifdef PLATFORMUNIX
#ifdef PLATFORMOSX
#include <unistd.h>
#include <mach-o/dyld.h>
#include <sys/wait.h>
#else /* DEBIAN */
#define _XOPEN_SOURCE 600
#include <unistd.h>
#include <libgen.h>
#include <sys/wait.h>
#endif
#else /* WINDOWS */
#include <windows.h>
#include <psapi.h>
#include <shlwapi.h>
#include <sys/stat.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef SELF_PATCH_ENABLED
#if STEAM_ENABLED
#define SELF_PATCH_ENABLED false
#else
#define SELF_PATCH_ENABLED true
#endif
#endif

#ifdef PLATFORMUNIX
#define GAMEPATH "./bin/game"
#else
#define GAMEPATH ".\\bin\\game.exe"
#endif

#if SELF_PATCH_ENABLED
#ifndef PLATFORMUNIX
#define OLDLISTPATH ".\\downloads\\old.list"
#endif
#endif

#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

#define MAXLEN 4096
#define BUFFERLEN 4098


int main(int argc, char* argv[])
{
#ifdef PLATFORMUNIX
#ifdef PLATFORMOSX
    {
        unsigned int bufferSize = 512;
        char* buffer = malloc((bufferSize + 1) * sizeof(char));
        if (_NSGetExecutablePath(&buffer[0], &bufferSize) != 0)
        {
            if (!realloc(buffer, bufferSize)) return 1;
            _NSGetExecutablePath(&buffer[0], &bufferSize);
        }
        strrchr(&buffer[0], '/')[0] = '\0';
        chdir(buffer);
        free(buffer);
    }
#else /* DEBIAN */
    {
        char path[BUFFERLEN];
        ssize_t size = readlink("/proc/self/exe", path, MAXLEN);
        if (size <= 0 || size >= MAXLEN)
        {
            fprintf(stderr, "Failed to read executable path.\n");
        }
        path[size] = '\0';
        if (chdir(dirname(path)))
        {
            fprintf(stderr, "Failed to change working directory.\n");
        }
    }
#endif
#else /* WINDOWS */
    {
        wchar_t path[BUFFERLEN];
        GetModuleFileNameExW(GetCurrentProcess(), NULL, path, MAXLEN);
        PathRemoveFileSpecW(path);
        SetCurrentDirectoryW(path);
    }
#endif

#if SELF_PATCH_ENABLED
#ifndef PLATFORMUNIX
	// If there were any live binaries during patching, remove them now.
	// We cannot do this in the game or before restarting the game because
	// the launcher itself might be one of the files being removed, and
	// in Windows binaries cannot or should not unlink themselves while running.
	{
		FILE* oldlist;
		if ((oldlist = fopen(OLDLISTPATH, "r")))
		{
			char path[MAX_PATH + 2];
			while (fgets(path, sizeof(path), oldlist))
			{
				size_t pathlen = strcspn(path, " \t\r\n");
				path[pathlen] = '\0';

				if (!(pathlen > 9 + 1 + 4
					&& strncmp(path, "downloads", 9) == 0
					&& strncmp(path + pathlen - 4, ".old", 4) == 0))
				{
					continue;
				}

				if (strstr(path, "..") != NULL)
				{
					continue;
				}

				struct stat buffer;
				if (stat(path, &buffer) != 0 || !S_ISREG(buffer.st_mode))
				{
					continue;
				}

				DeleteFileA(path);
			}

			fclose(oldlist);
		}

		if ((oldlist = fopen(OLDLISTPATH, "w")))
		{
			fclose(oldlist);
		}
	}
#endif
#endif

	char gamecommand[BUFFERLEN] = "";
	strcat(gamecommand, GAMEPATH);
	strcat(gamecommand, " launcher=1");

	// Pass the command line arguments along to the game.
	for (int i = 1; i < argc; i++)
	{
		if (strlen(gamecommand) + 1 + strlen(argv[i]) >= MAXLEN) break;
		strcat(gamecommand, " ");
		strcat(gamecommand, argv[i]);
	}

	for (int i = 0; i < 1000; i++)
	{
		int status = system(gamecommand);

#ifdef PLATFORMUNIX
		if (status < 0)
		{
			fprintf(stderr, "Failed to start game.\n");
			return 1;
		}
		else if (!WIFEXITED(status))
		{
			fprintf(stderr, "Game crashed.\n");
			return 1;
		}
		else
		{
			status = WEXITSTATUS(status);
		}
#else
		if (status < 0)
		{
			fprintf(stderr, "Failed to start game, or game crashed.\n");
			return 1;
		}
#endif

		if (status == 0)
		{
			// Done.
			return 0;
		}
		else if (status == 2)
		{
			// The game has requested a restart. Continue below.
		}
		else
		{
			fprintf(stderr, "Game crashed (exit status: %d).\n", status);
			return 1;
		}

		// Restart the game.
	}
}
