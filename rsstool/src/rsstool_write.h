/*
rsstool_write.h - definitions for RSStool

Copyright (c) 2006 NoisyB


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef RSSTOOL_WRITE_H
#define RSSTOOL_WRITE_H


/*
  write feed contents

  rsstool_write_xml()
  rsstool_write_rss()       as RSS feed
*/
extern int rsstool_write_xml (st_rsstool_t *rt);
extern int rsstool_write_rss (st_rsstool_t *rt, int type);


#endif  // RSSTOOL_WRITE_H
