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

#include <config.h>
#include "buffer_ssl.h"

#if USE_SSL
# include "lftp_ssl.h"

// IOBufferSSL implementation
#undef super
#define super IOBuffer

int IOBufferSSL::Do()
{
   if(Done() || Error())
      return STALL;

   int res=0;

   switch(mode)
   {
   case PUT:
      if(Size()==0 && ssl->handshake_done)
	 return STALL;
      res=Put_LL(buffer+buffer_ptr,Size());
      if(res>0)
      {
	 buffer_ptr+=res;
	 event_time=now;
	 if(eof)
	    PutEOF_LL();
	 return MOVED;
      }
      break;

   case GET:
      if(eof)
	 return STALL;
      res=Get_LL(GET_BUFSIZE);
      if(res>0)
      {
	 EmbraceNewData(res);
	 event_time=now;
	 return MOVED;
      }
      if(eof)
      {
	 event_time=now;
	 return MOVED;
      }
      break;
   }
   if(res<0)
   {
      event_time=now;
      return MOVED;
   }
   if(ssl->want_in())
      Block(ssl->fd,POLLIN);
   if(ssl->want_out())
      Block(ssl->fd,POLLOUT);
   return STALL;
}

int IOBufferSSL::Get_LL(int size)
{
   int res=ssl->read(GetSpace(size),size);
   if(res<0)
   {
      if(res==ssl->RETRY)
	 return 0;
      else // error
      {
	 SetError(ssl->error,ssl->fatal);
	 return -1;
      }
   }
   if(res==0)
      eof=true;
   return res;
}

int IOBufferSSL::Put_LL(const char *buf,int size)
{
   int res=ssl->write(buf,size);
   if(res<0)
   {
      if(res==ssl->RETRY)
	 return 0;
      else // error
      {
	 SetError(ssl->error,ssl->fatal);
	 return -1;
      }
   }
   return res;
}

int IOBufferSSL::PutEOF_LL()
{
   if(Size()==0)
      ssl->shutdown();
   return 0;
}

IOBufferSSL::~IOBufferSSL()
{
}

#endif // USE_SSL
