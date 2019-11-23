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
};

struct commands {
	void *context;
	struct command *list;
};

static int command_empty_or_null(struct command *command)
{
	return command == NULL || command->name == NULL || command->function == NULL;
}

static int commands_empty_or_null(struct commands *commands)
{
	return commands == NULL || command_empty_or_null(commands->list);
}

static int string_empty_or_null(const char *string)
{
	return string == NULL || *string == '\0';
}

static int vector_empty_or_null(const char **vector)
{
	return vector == NULL || string_empty_or_null(*vector);
}

static struct command *find(struct command *commands, const char *line)
{
	for (; commands->name != NULL; ++commands) {
		size_t command_length = strlen(commands->name);

		if (strncmp(commands->name, line, command_length) == 0)
			return commands;
	}

	return NULL;
}

int process_command_vector(struct commands *commands, const char **arguments)
{
	if (vector_empty_or_null(arguments) || commands_empty_or_null(commands))
		return 0; // NULL or empty inputs

	struct command *command = find(commands->list, *arguments++);
	if (command == NULL)
		return -1; // command not found

	size_t count = 0;
	for (const char **p = arguments; *p != NULL; ++p, ++count);

	return command->function(count, arguments, commands->context);
}

static int process_command_line_copy(struct commands *commands, char *copy, size_t length)
{
	int result = 0;

	// Strings will be in the form of interleaved space/non-space characters
	// For example: "a b c d"
	// For a string of length N, at least ceil(N / 2) pointers are required
	// to point at all possible tokens, plus one for the trailing NULL
	const char **arguments = calloc(DIV_CEIL(length, 2) + 1, sizeof(*arguments));
	if (arguments == NULL) {
		result = -2; // memory allocation error
		goto free_and_exit;
	}

	copy = strtok(copy, " \t\n");
	if (copy == NULL) {
		result = 0; // empty line
		goto free_and_exit;
	}

	struct command *command = find(commands->list, copy);
	if (command == NULL) {
		result = -1; // command not found
		goto free_and_exit;
	}

	size_t i = 0;
	for (char *argument = copy; argument != NULL; ++i)
		arguments[i] = argument = strtok(NULL, " \t\n");
	arguments[i] = NULL;

	result = command->function(i - 1, arguments, commands->context);


free_and_exit:
	free(arguments);
	return result;
}

int process_command_line(struct commands *commands, const char *line)
{
	if (string_empty_or_null(line) || commands_empty_or_null(commands))
		return 0; // NULL or empty inputs

	int result = 0;

	// strtok will modify the string so make a copy first
	size_t length = strlen(line);
	char *copy = malloc(length + 1);
	if (copy == NULL) {
		result = -2; // memory allocation error
		goto free_and_exit;
	}
	strcpy(copy, line);

	result = process_command_line_copy(commands, copy, length);

free_and_exit:
	free(copy);
	return result;
}

int process_command_file(struct commands *commands, FILE *input)
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
