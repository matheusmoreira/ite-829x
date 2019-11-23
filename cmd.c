// SPDX-License-Identifier: GPL-2.0-only

/* Copyright © 2019 Matheus Afonso Martins Moreira
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define DIV_CEIL(x, y) ((((x) - 1) / (y)) + 1)

typedef unsigned int (*command_function)(size_t count, const char **arguments, void *context);

struct command {
	const char *name;
	command_function function;
	void *context;
};

static struct command *find(struct command *commands, const char *line)
{
	for (; commands->name != NULL; ++commands) {
		size_t command_length = strlen(commands->name);

		if (strncmp(commands->name, line, command_length) == 0)
			return commands;
	}

	return NULL;
}

int process_command_vector(struct command *commands, const char **arguments)
{
	if (commands == NULL || commands->name == NULL || arguments == NULL || *arguments == NULL)
		return 0; // NULL or empty inputs

	struct command *command = find(commands, *arguments++);
	if (command == NULL)
		return -1; // command not found

	size_t count = 0;
	for (const char **p = arguments; *p != NULL; ++p, ++count);

	return command->function(count, arguments, command->context);
}

int process_command_line(struct command *commands, const char *line)
{
	if (commands == NULL || commands->name == NULL || line == NULL || *line == '\0')
		return 0; // NULL or empty inputs

	int result = 0;

	// strtok will modify the string so make a copy first
	size_t length = strlen(line);
	char *copy = malloc(length + 1);
	if (copy == NULL) {
		result = -2; // memory allocation error
		goto free_copy;
	}
	strcpy(copy, line);

	// Strings will be in the form of interleaved space/non-space characters
	// For example: "a b c d"
	// For a string of length N, at least ceil(N / 2) pointers are required
	// to point at all possible tokens, plus one for the trailing NULL
	const char **arguments = calloc(DIV_CEIL(length, 2) + 1, sizeof(*arguments));
	if (arguments == NULL) {
		result = -2; // memory allocation error
		goto free_copy_arguments;
	}

	copy = strtok(copy, " \t\n");
	if (copy == NULL) {
		result = 0; // empty line
		goto free_copy_arguments;
	}

	struct command *command = find(commands, copy);
	if (command == NULL) {
		result = -1; // command not found
		goto free_copy_arguments;
	}

	size_t i = 0;
	for (char *argument = copy; argument != NULL; ++i)
		arguments[i] = argument = strtok(NULL, " \t\n");
	arguments[i] = NULL;
	result = command->function(i - 1, arguments, command->context);

free_copy_arguments:
	free(arguments);
free_copy:
	free(copy);
	return result;
}

int process_command_file(struct command *commands, FILE *input)
{
	int result = 0;

	while (1) {
		char *buffer = NULL;
		size_t length = 0;

		errno = 0;
		ssize_t read = getline(&buffer, &length, input);

		if (read == -1) {
			if (errno)
				result = -2; // memory allocation error

			goto free;
		}

		result = process_command_line(commands, buffer);

free:
		free(buffer);

		if (result || read == -1)
			break;
	}

	return result;
}
