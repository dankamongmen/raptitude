// \file terminal.h                -*-c++-*-
//
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

#ifndef APTITUDE_CMDLINE_MOCKS_TERMINAL_H
#define APTITUDE_CMDLINE_MOCKS_TERMINAL_H

// Local includes:
#include <cmdline/terminal.h>

// System includes:
#include <boost/shared_ptr.hpp>

#include <gmock/gmock.h>

namespace boost
{
  template<typename T> boost::shared_ptr<T> make_shared();
}

namespace aptitude
{
  namespace cmdline
  {
    namespace mocks
    {
      /** \brief A mock terminal object.
       *
       *  This object doesn't have any special behavior; it can be
       *  used to test other mocks that extend the terminal's behavior
       *  (such as the teletype mock).  To get a terminal that
       *  interprets calls to write_text(), use
       *  combining_terminal_output.
       */
      class terminal_output : public aptitude::cmdline::terminal_output
      {
        friend boost::shared_ptr<terminal_output>
        boost::make_shared<terminal_output>();

        terminal_output();

      public:
        MOCK_METHOD0(output_is_a_terminal, bool());
        MOCK_METHOD1(write_text, void(const std::wstring &));
        MOCK_METHOD0(move_to_beginning_of_line, void());
        MOCK_METHOD0(flush, void());

        static boost::shared_ptr<terminal_output> create();
      };

      class terminal_input : public aptitude::cmdline::terminal_input
      {
        friend boost::shared_ptr<terminal_input>
        boost::make_shared<terminal_input>();

        terminal_input();

      public:
        MOCK_METHOD1(prompt_for_input, std::wstring(const std::wstring &));

        static boost::shared_ptr<terminal_input> create();
      };

      class terminal_metrics : public aptitude::cmdline::terminal_metrics
      {
        friend boost::shared_ptr<terminal_metrics>
        boost::make_shared<terminal_metrics>();

        terminal_metrics();

      public:
        MOCK_METHOD0(get_screen_width, unsigned int());

        static boost::shared_ptr<terminal_metrics> create();
      };

      /** \brief A mock for the terminal locale routines.
       *
       *  By default, returns 1 from every call to wcwidth() and marks
       *  those calls as expected.
       */
      class terminal_locale : public aptitude::cmdline::terminal_locale
      {
        friend boost::shared_ptr<terminal_locale>
        boost::make_shared<terminal_locale>();

        terminal_locale();

      public:
        MOCK_METHOD1(wcwidth, int(wchar_t));

        static boost::shared_ptr<terminal_locale> create();
      };

      /** \brief Interface for objects that emit terminal output as a
       *  sequence of string writes.
       */
      class terminal_with_combined_output
      {
        terminal_with_combined_output();

        friend class combining_terminal_output;

        friend boost::shared_ptr<terminal_with_combined_output>
        boost::make_shared<terminal_with_combined_output>();

      public:
        virtual ~terminal_with_combined_output();

        // This method is invoked when the terminal would flush its
        // output: specifically, for each newline that's written and
        // for each call to flush().
        //
        // If the terminal would flush, but there's no text to flush,
        // this isn't invoked.
        MOCK_METHOD1(output, void(const std::wstring &));

        static boost::shared_ptr<terminal_with_combined_output> create();
      };

      /** \brief Interface for objects that can receive calls to
       *  terminal_output and emit calls as a
       *  terminal_with_combined_output.
       *
       *  Calls to move_to_beginning_of_line() are rewritten to place
       *  '\r' on the output stream instead.  This is done so that
       *  client code can easily verify that the
       *  move_to_beginning_of_line() occurs in the right place
       *  relative to calls to write_to_text().
       */
      class combining_terminal_output : public aptitude::cmdline::terminal_output,
                                        public terminal_with_combined_output
      {
        class impl;
        friend class impl;

        friend boost::shared_ptr<combining_terminal_output>
        create_combining_terminal_output();

        combining_terminal_output();

      public:
        // Mocked because tests might want to override its behavior:
        MOCK_METHOD0(output_is_a_terminal, bool());

        static boost::shared_ptr<combining_terminal_output> create();
      };
    }
  }
}

#endif
