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

#ifndef LOG_H
#define LOG_H

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include "SMTask.h"

class Log : public SMTask
{
   int output;
   bool need_close_output;
   bool tty;
   bool show_pid;
   bool show_time;
   bool show_context;
   bool at_line_start;
   typedef void (*tty_cb_t)();
   tty_cb_t tty_cb;

   void CloseOutput()
      {
	 if(need_close_output)
	    close(output);
	 output=-1;
	 need_close_output=false;
      }

   bool enabled;
   int level;

public:
   static SMTaskRef<Log> global;

   bool WillOutput(int l);
   void DoWrite(const char *str);
   void Write(int l,const char *str);
   void Format(int l,const char *fmt,...) PRINTF_LIKE(3,4);
   void vFormat(int l,const char *fmt,va_list v);

   void SetLevel(int l) { level=l; }
   void Enable()  { enabled=true;  }
   void Disable() { enabled=false; }

   void SetOutput(int o,bool need_close);

   void SetCB(tty_cb_t cb) { tty_cb=cb; }

   bool IsTTY() { return tty; }

   void Init();
   Log() { Init(); }
   ~Log();

   int Do();

   void ShowPID(bool yes=true) { show_pid=yes; }
   void ShowTime(bool yes=true) { show_time=yes; }
   void ShowContext(bool yes=true) { show_context=yes; }
   void ShowNothing() { show_pid=show_time=show_context=false; }

   static void Cleanup();
};

#endif // LOG_H
