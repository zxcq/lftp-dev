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
#include "SSH_Access.h"
#include "misc.h"

void SSH_Access::MakePtyBuffers()
{
   int fd=ssh->getfd();
   if(fd==-1)
      return;
   ssh->Kill(SIGCONT);
   send_buf=new IOBufferFDStream(new FDStream(ssh->getfd_pipe_out(),"pipe-out"),IOBuffer::PUT);
   recv_buf=new IOBufferFDStream(new FDStream(ssh->getfd_pipe_in(),"pipe-in"),IOBuffer::GET);
   pty_send_buf=new IOBufferFDStream(ssh.borrow(),IOBuffer::PUT);
   pty_recv_buf=new IOBufferFDStream(new FDStream(fd,"pseudo-tty"),IOBuffer::GET);
}

int SSH_Access::HandleSSHMessage()
{
   int m=STALL;
   const char *b;
   int s;
   pty_recv_buf->Get(&b,&s);
   const char *eol=find_char(b,s,'\n');
   if(!eol)
   {
      const char *p="password:";
      const char *p_for="password for ";
      const char *y="(yes/no)?";
      int p_len=strlen(p);
      int p_for_len=strlen(p_for);
      int y_len=strlen(y);
      if(s>0 && b[s-1]==' ')
	 s--;
      if((s>=p_len && !strncasecmp(b+s-p_len,p,p_len))
      || (s>10 && !strncmp(b+s-2,"':",2))
      || (s>p_for_len && b[s-1]==':' && !strncasecmp(b,p_for,p_for_len)))
      {
	 if(!pass)
	 {
	    SetError(LOGIN_FAILED,_("Password required"));
	    return MOVED;
	 }
	 if(password_sent>0)
	 {
	    SetError(LOGIN_FAILED,_("Login incorrect"));
	    return MOVED;
	 }
	 pty_recv_buf->Put("XXXX");
	 pty_send_buf->Put(pass);
	 pty_send_buf->Put("\n");
	 password_sent++;
	 return m;
      }
      if(s>=y_len && !strncasecmp(b+s-y_len,y,y_len))
      {
	 pty_recv_buf->Put("yes\n");
	 pty_send_buf->Put("yes\n");
	 return m;
      }
      if(!received_greeting && recv_buf->Size()>0)
      {
	 recv_buf->Get(&b,&s);
	 eol=find_char(b,s,'\n');
	 if(eol)
	 {
	    xstring &line=xstring::get_tmp(b,eol-b);
	    if(line.eq(greeting))
	       received_greeting=true;
	    LogRecv(4,line);
	    recv_buf->Skip(eol-b+1);
	 }
      }
      LogSSHMessage();
      return m;
   }
   const char *f=N_("Host key verification failed");
   if(!strncasecmp(b,f,strlen(f)))
   {
      LogSSHMessage();
      SetError(FATAL,_(f));
      return MOVED;
   }
   if(eol>b && eol[-1]=='\r')
      eol--;
   f=N_("Name or service not known");
   int f_len=strlen(f);
   if(eol-b>=f_len && !strncasecmp(eol-f_len,f,f_len)) {
      LogSSHMessage();
      SetError(LOOKUP_ERROR,xstring::get_tmp(b,eol-b));
      return MOVED;
   }
   LogSSHMessage();
   return MOVED;
}

void SSH_Access::LogSSHMessage()
{
   const char *b;
   int s;
   pty_recv_buf->Get(&b,&s);
   const char *eol=find_char(b,s,'\n');
   if(!eol)
   {
      if(pty_recv_buf->Eof())
      {
	 if(s>0)
	    LogRecv(4,b);
	 LogError(0,_("Peer closed connection"));
      }
      if(pty_recv_buf->Error())
	 LogError(4,"pty read: %s",pty_recv_buf->ErrorText());
      if(pty_recv_buf->Eof() || pty_recv_buf->Error()) {
	 if(last_ssh_message && time_t(now)-last_ssh_message_time<4)
	    LogError(0,"%s",last_ssh_message.get());
	 Disconnect(last_ssh_message);
      }
      return;
   }
   s=eol-b+1;
   int chomp_cr=(s>=2 && b[s-2]=='\r');
   last_ssh_message.nset(b,s-1-chomp_cr);
   last_ssh_message_time=now;
   pty_recv_buf->Skip(s);
   LogRecv(4,last_ssh_message);
   if(last_ssh_message.begins_with("ssh: "))
      last_ssh_message.set(last_ssh_message+5);

   if(!received_greeting && last_ssh_message.eq(greeting))
      received_greeting=true;
}

void SSH_Access::DisconnectLL()
{
   if(send_buf)
      LogNote(9,_("Disconnecting"));
   send_buf=0;
   recv_buf=0;
   pty_send_buf=0;
   pty_recv_buf=0;
   ssh=0;
   received_greeting=false;
   password_sent=0;
   last_ssh_message.unset();
   last_ssh_message_time=0;
}

void SSH_Access::MoveConnectionHere(SSH_Access *o)
{
   send_buf=o->send_buf.borrow();
   recv_buf=o->recv_buf.borrow();
   pty_send_buf=o->pty_send_buf.borrow();
   pty_recv_buf=o->pty_recv_buf.borrow();
   ssh=o->ssh.borrow();
   received_greeting=o->received_greeting;
   password_sent=o->password_sent;
   last_ssh_message.move_here(o->last_ssh_message);
   last_ssh_message_time=o->last_ssh_message_time; o->last_ssh_message_time=0;
}
