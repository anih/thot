/*
thot package for statistical machine translation
Copyright (C) 2017 Adam Harasimowicz

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; If not, see <http://www.gnu.org/licenses/>.
*/

/********************************************************************/
/*                                                                  */
/* Module: StdCerrThreadSafePrint.h                                 */
/*                                                                  */
/* Prototypes file: StdCerrThreadSafePrint.h                        */
/*                                                                  */
/* Description: Declares the StdCerrThreadSafePrint class which     */
/*              wraps ThreadSafePrint class to allow printing with  */
/*              << operator like in standard library.               */
/*                                                                  */
/********************************************************************/

/**
 * @file StdCerrThreadSafePrint.h
 *p
 * @brief Declares the StdCerrThreadSafePrint class which wraps
 * ThreadSafePrint class to allow printing with << operator like
 * in standard library.
 */

#ifndef _StdCerrThreadSafePrint_h
#define _StdCerrThreadSafePrint_h

#define StdCerrThreadSafe StdCerrThreadSafePrint{}

//--------------- Include files --------------------------------------

#if HAVE_CONFIG_H
#  include <thot_config.h>
#endif /* HAVE_CONFIG_H */

#include "ThreadSafePrint.h"
#include <sstream>

//--------------- Constants ------------------------------------------


//--------------- typedefs -------------------------------------------


//--------------- Classes --------------------------------------------

//--------------- StdCerrThreadSafePrint template class

/**
 * @brief Class implementing thread-safe printing
 */

class StdCerrThreadSafePrint : public std::ostringstream
{
public:
    StdCerrThreadSafePrint() {};

    ~StdCerrThreadSafePrint()
    {
        ThreadSafePrint::getInstance().print(this->str());
    }
};

#endif
