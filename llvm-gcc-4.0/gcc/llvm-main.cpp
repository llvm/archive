/* llvm-main.cpp: defines a C++ version of main() for cc1, cc1plus, etc.
Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
Contributed by Chris Lattner (sabre@nondot.org)

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#include <iostream>

extern "C" {
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "toplev.h"
}

// We define main() to call toplev_main(), which is defined in toplev.c.
// We do this so that C++ code has its static constructors called as required.
//

int
main (int argc, char **argv)
{
  return toplev_main (argc, (const char **) argv);
}
