/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  DataDifferential Utility Library
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "config.h"

#include "util/pidfile.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

namespace datadifferential {
namespace util {

Pidfile::Pidfile(const std::string &arg) :
  _last_errno(0),
  _filename(arg)
{
}


Pidfile::~Pidfile()
{
  if (not _filename.empty() and (unlink(_filename.c_str()) == -1))
  {
    _error_message+= "Could not remove the pid file: ";
    _error_message+= _filename;
  }
}

bool Pidfile::create()
{
  if (_filename.empty())
    return true;

  int file;
  if ((file = open(_filename.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU|S_IRGRP|S_IROTH)) < 0)
  {
    _error_message+= "Could not open pid file for writing: "; 
    _error_message+= _filename;
    return false;
  }

  char buffer[BUFSIZ];

  unsigned long temp= static_cast<unsigned long>(getpid());
  int length= snprintf(buffer, sizeof(buffer), "%lu\n", temp);

  if (write(file, buffer, length) != length)
  { 
    _error_message+= "Could not write pid to file: "; 
    _error_message+= _filename;

    return false;
  }

  if (close(file < 0))
  {
    _error_message+= "Could not close() file after writing pid to it: "; 
    _error_message+= _filename;
    return false;
  }

  return true;
}

} /* namespace util */
} /* namespace datadifferential */
