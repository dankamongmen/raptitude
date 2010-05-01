/** \file progress_info.h */     // -*-c++-*-

// Copyright (C) 2010 Daniel Burrows
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.

#ifndef PROGRESS_INFO_H
#define PROGRESS_INFO_H

#include <string>

namespace aptitude
{
  namespace util
  {
    /** \brief Data types related to progress information. */
    // @{

    /** \brief The type of progress information being stored.
     */
    enum progress_type
      {
        /** \brief There is no progress information. */
        progress_type_none,
        /** \brief There is only a progress pulse. */
        progress_type_pulse,
        /** \brief Full progress information is available. */
        progress_type_bar
      };

    class progress_info
    {
      progress_type type;
      double progress_fraction;
      std::string progress_status;

      progress_info(double _progress_fraction,
                    std::string _progress_status)
        : type(progress_type_bar),
          progress_fraction(_progress_fraction),
          progress_status(_progress_status)
      {
      }

      progress_info(progress_type _type)
        : type(_type), progress_fraction(0)
      {
      }

    public:
      static progress_info none()
      {
        return progress_info(progress_type_none);
      }

      static progress_info pulse()
      {
        return progress_info(progress_type_pulse);
      }

      static progress_info bar(double fraction,
                               const std::string &status)
      {
        return progress_info(fraction, status);
      }
    };

    // @}
  }
}

#endif // PROGRESS_INFO_H