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
 * Peek and poke command self-tests
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
#include <ipxe/test.h>
#include <ipxe/uaccess.h>

/**
 * Perform peek and poke self-tests
 *
 */
static void mem_cmd_test_exec ( void ) {
	uint8_t buffer[4] = { 0x00, 0x11, 0x22, 0x33 };
	physaddr_t phys = virt_to_phys ( buffer );
	char addr_str[32];
	char *poke_argv[] = { "poke", addr_str, "0xba", NULL };
	char *peek_argv[] = { "peek", addr_str, NULL };
	int rc;

	snprintf ( addr_str, sizeof ( addr_str ), "0x%lx", ( unsigned long ) phys );

	/* Execute poke */
	rc = execv ( "poke", poke_argv );
	ok ( rc == 0 );
	ok ( buffer[0] == 0xba );

	/* Execute peek */
	rc = execv ( "peek", peek_argv );
	ok ( rc == 0 );
}

/** Peek and poke command self-test */
struct self_test mem_cmd_test __self_test = {
	.name = "mem_cmd",
	.exec = mem_cmd_test_exec,
};
