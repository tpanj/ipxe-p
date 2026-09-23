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
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <ipxe/uaccess.h>
#include <ipxe/command.h>
#include <ipxe/parseopt.h>
#include <ipxe/image.h>
#include <usr/imgmgmt.h>

FILE_LICENCE ( GPL2_OR_LATER_OR_UBDL );
FILE_SECBOOT ( PERMITTED );

/** @file
 *
 * Peek, poke, and memfetch memory commands
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

/** "memfetch" options */
struct memfetch_options {
	/** Image name */
	char *name;
	/** Download timeout */
	unsigned long timeout;
	/** Unregister image after execution */
	int autofree;
	/** Memory address to copy module data to */
	unsigned long address;
	/** Flag indicating if address option was specified */
	int has_address;
};

/**
 * Parse address option for memfetch
 *
 * @v text		Text
 * @v address		Address field in memfetch options
 * @ret rc		Return status code
 */
static int parse_address_opt ( char *text, unsigned long *address ) {
	struct memfetch_options *opts =
		container_of ( address, struct memfetch_options, address );
	int rc;

	if ( ( rc = parse_address ( text, address ) ) != 0 )
		return rc;

	opts->has_address = 1;
	return 0;
}

/** "memfetch" option list */
static struct option_descriptor memfetch_opts[] = {
	OPTION_DESC ( "name", 'n', required_argument,
		      struct memfetch_options, name, parse_string ),
	OPTION_DESC ( "timeout", 't', required_argument,
		      struct memfetch_options, timeout, parse_timeout ),
	OPTION_DESC ( "autofree", 'a', no_argument,
		      struct memfetch_options, autofree, parse_flag ),
	OPTION_DESC ( "address", 'A', required_argument,
		      struct memfetch_options, address, parse_address_opt ),
};

/** "memfetch" command descriptor */
static struct command_descriptor memfetch_cmd =
	COMMAND_DESC ( struct memfetch_options, memfetch_opts, 1, MAX_ARGUMENTS,
		       "<uri> [<arguments>...]" );

/**
 * "memfetch" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Return status code
 */
static int memfetch_exec ( int argc, char **argv ) {
	struct memfetch_options opts;
	char *name_uri = NULL;
	char *cmdline = NULL;
	struct image *image;
	int rc;

	/* Parse options */
	if ( ( rc = parse_options ( argc, argv, &memfetch_cmd, &opts ) ) != 0 )
		goto err_parse_options;

	/* Parse name/URI string and command line, if present */
	if ( optind < argc ) {
		name_uri = argv[optind];
		if ( argv[ optind + 1 ] != NULL ) {
			cmdline = concat_args ( &argv[ optind + 1 ] );
			if ( ! cmdline ) {
				rc = -ENOMEM;
				goto err_parse_cmdline;
			}
		}
	}

	/* Acquire the image */
	if ( name_uri ) {
		if ( ( rc = imgacquire ( name_uri, opts.timeout,
					 &image ) ) != 0 )
			goto err_acquire;
	} else {
		printf ( "Missing URI\n" );
		rc = -EINVAL;
		goto err_acquire;
	}

	/* Set the image name, if applicable */
	if ( opts.name ) {
		if ( ( rc = image_set_name ( image, opts.name ) ) != 0 ) {
			printf ( "Could not name image: %s\n",
				 strerror ( rc ) );
			goto err_set_name;
		}
	}

	/* Set the command-line arguments, if applicable */
	if ( cmdline ) {
		if ( ( rc = image_set_cmdline ( image, cmdline ) ) != 0 ) {
			printf ( "Could not set arguments: %s\n",
				 strerror ( rc ) );
			goto err_set_cmdline;
		}
	}

	/* Set the auto-unregister flag, if applicable */
	if ( opts.autofree )
		image->flags |= IMAGE_AUTO_UNREGISTER;

	/* Copy fetched module data to address if specified */
	if ( opts.has_address ) {
		memcpy ( phys_to_virt ( opts.address ), image->data,
			 image->len );
	}

	rc = 0;

 err_set_cmdline:
 err_set_name:
 err_acquire:
	free ( cmdline );
 err_parse_cmdline:
 err_parse_options:
	return rc;
}

/** Memory commands */
COMMAND ( peek, peek_exec );
COMMAND ( poke, poke_exec );
COMMAND ( memfetch, memfetch_exec );
