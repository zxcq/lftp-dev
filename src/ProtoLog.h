/*
 * lftp - file transfer program
 *
 * Copyright (c) 1996-2012 by Alexander V. Lukyanov (lav@yars.free.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROTOLOG_H
#define PROTOLOG_H

class ProtoLog
{
   static bool WillOutput(int level);
public:
   static void Log2(int level,xstring& str);
   static void Log3(int level,const char *prefix,const char *str);
   static void LogError(int level,const char *fmt,...) PRINTF_LIKE(2,3);
   static void LogNote(int level,const char *fmt,...) PRINTF_LIKE(2,3);
   static void LogRecv(int level,const char *line);
   static void LogSend(int level,const char *line);
};

#endif//PROTOLOG_H
