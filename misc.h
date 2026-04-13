   // ----------------------------------------------------------------------------
// misc.h  --  Miscellaneous helper functions
//
// Copyright (C) 2006-2008
//		Dave Freese, W1HKJ
//
// This file is part of fldigi.  These filters were adapted from code contained
// in the gmfsk source code distribution.
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#ifndef _MISC_H
#define _MISC_H

#include <cmath>
inline double sinc(double x)
{
	return (fabs(x) < 1e-10) ? 1.0 : (sin(M_PI * x) / (M_PI * x));
}
inline double decayavg(double average, double input, int weight)
{
	if (weight <= 1) return input;
	return ( ( input - average ) / (double)weight ) + average ;
}
#endif