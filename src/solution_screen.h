// solution_screen.h                          -*-c++-*-
//
//   Copyright (C) 2005 Daniel Burrows
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2 of
//   the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; see the file COPYING.  If not, write to
//   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//   Boston, MA 02111-1307, USA.

#ifndef SOLUTION_SCREEN_H
#define SOLUTION_SCREEN_H

#include <vscreen/cwidget::util::ref_ptr.h>

class aptitude_universe;
template<class PackageUniverse> class generic_solution;
class cwidget::widgets::widget;

typedef generic_solution<aptitude_universe> aptitude_solution;
typedef cwidget::util::ref_ptr<cwidget::widgets::widget> cwidget::widgets::widget_ref;

/** Generate a new widget suitable for inclusion at the top-level
 *  which describes the given solution.
 */
cwidget::widgets::widget_ref make_solution_screen();

#endif // SOLUTION_SCREEN_H
