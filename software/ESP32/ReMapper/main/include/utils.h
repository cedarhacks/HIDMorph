#pragma once

#include <stdio.h>
#include <string.h>
#include <dirent.h>

int files_in_dir(const char* path, char* output[255], int output_size);