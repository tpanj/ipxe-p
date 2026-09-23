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

FILE_LICENCE ( GPL2_OR_LATER_OR_UBDL );

/** @file
 *
 * Peek, poke, and memfetch command self-tests
 *
 */

/* Forcibly enable assertions */
#undef NDEBUG

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ipxe/command.h>
#include <ipxe/image.h>
#include <ipxe/test.h>
#include <ipxe/uaccess.h>

/**
 * Perform peek, poke, and memfetch self-tests
 *
 */
static void mem_cmd_test_exec ( void ) {
	uint8_t src_buffer[4] = { 'A', 'B', 'C', 'D' };
	uint8_t buffer[16] = { 0 };
	physaddr_t phys = virt_to_phys ( buffer );
	char addr_str[32];
	char *poke_argv[] = { "poke", addr_str, "0xba", NULL };
	char *peek_argv[] = { "peek", addr_str, NULL };
	char *memfetch_argv[] = { "memfetch", "-A", addr_str, "test_img", NULL };
	struct image *image;
	int rc;

	snprintf ( addr_str, sizeof ( addr_str ), "0x%lx", ( unsigned long ) phys );

	/* Execute poke */
	rc = execv ( "poke", poke_argv );
	ok ( rc == 0 );
	ok ( buffer[0] == 0xba );

	/* Execute peek */
	rc = execv ( "peek", peek_argv );
	ok ( rc == 0 );

	/* Create image in memory */
	image = image_memory ( "test_img", src_buffer, sizeof ( src_buffer ) );
	ok ( image != NULL );

	if ( image ) {
		/* Execute memfetch */
		rc = execv ( "memfetch", memfetch_argv );
		ok ( rc == 0 );
		ok ( memcmp ( buffer, "ABCD", 4 ) == 0 );

		/* Unregister test image */
		unregister_image ( image );
	}
}

/** Peek, poke, and memfetch command self-test */
struct self_test mem_cmd_test __self_test = {
	.name = "mem_cmd",
	.exec = mem_cmd_test_exec,
};
