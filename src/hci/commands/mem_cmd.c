/*
 * Copyright (C) 2023 iPXE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * You can also choose to distribute this program under the terms of
 * the Unmodified Binary Distribution Licence (as given in the file
 * COPYING.UBDL), provided that you have satisfied its requirements.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <ipxe/uaccess.h>
#include <ipxe/command.h>
#include <ipxe/parseopt.h>

FILE_LICENCE ( GPL2_OR_LATER_OR_UBDL );
FILE_SECBOOT ( PERMITTED );

/** @file
 *
 * Peek and poke memory commands
 *
 */

/** "peek" options */
struct peek_options {};

/** "peek" option list */
static struct option_descriptor peek_opts[] = {};

/** "peek" command descriptor */
static struct command_descriptor peek_cmd =
	COMMAND_DESC ( struct peek_options, peek_opts, 1, 1,
		       "<address>" );

/**
 * Parse address value
 *
 * @v text		Text
 * @ret value		Address value
 * @ret rc		Return status code
 */
static int parse_address ( char *text, unsigned long *value ) {
	char *endp;

	/* Sanity check */
	assert ( text != NULL );

	/* Parse address */
	*value = strtoul ( text, &endp, 0 );
	if ( *endp || ( ! *text ) ) {
		printf ( "\"%s\": invalid integer value\n", text );
		return -EINVAL;
	}

	return 0;
}

/**
 * "peek" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Return status code
 */
static int peek_exec ( int argc, char **argv ) {
	struct peek_options opts;
	unsigned long address;
	uint8_t *ptr;
	int rc;

	/* Parse options */
	if ( ( rc = parse_options ( argc, argv, &peek_cmd, &opts ) ) != 0 )
		return rc;

	/* Parse address */
	if ( ( rc = parse_address ( argv[optind], &address ) ) != 0 )
		return rc;

	/* Read byte from address */
	ptr = phys_to_virt ( address );
	printf ( "0x%02x\n", *ptr );

	return 0;
}

/** "poke" options */
struct poke_options {};

/** "poke" option list */
static struct option_descriptor poke_opts[] = {};

/** "poke" command descriptor */
static struct command_descriptor poke_cmd =
	COMMAND_DESC ( struct poke_options, poke_opts, 2, 2,
		       "<address> <value>" );

/**
 * "poke" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Return status code
 */
static int poke_exec ( int argc, char **argv ) {
	struct poke_options opts;
	unsigned long address;
	unsigned int value;
	uint8_t *ptr;
	int rc;

	/* Parse options */
	if ( ( rc = parse_options ( argc, argv, &poke_cmd, &opts ) ) != 0 )
		return rc;

	/* Parse address */
	if ( ( rc = parse_address ( argv[optind], &address ) ) != 0 )
		return rc;

	/* Parse value */
	if ( ( rc = parse_integer ( argv[optind + 1], &value ) ) != 0 )
		return rc;

	/* Write byte to address */
	ptr = phys_to_virt ( address );
	*ptr = ( uint8_t ) value;

	return 0;
}

/** Memory commands */
COMMAND ( peek, peek_exec );
COMMAND ( poke, poke_exec );
