/*
 * Copyright Thomas M. Grosen 
 * Created on 11/01/2024
 */

/*
 * This file is part of MoniTAal
 *
 * MoniTAal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MoniTAal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MoniTAal. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MONITAAL_EVENT_PARSER_H
#define MONITAAL_EVENT_PARSER_H

#include "TA.h"
#include "Monitor.h"
#include "types.h"
#include "errors.h"

#include <fstream>


/** EVENT PARSER
 *  Parse events (time points and labels) separated by '@' symbols.
 *
 *  Parses either concrete or interval times (never both), by calling the specific parsing function.
 *  Concrete means time points are singular decimals.
 *  Interval means time is bounded by an integer lower and upper bound.
 *
 *  Whitespace is ignored (space, tab, newline). 
 *  - Whitespace cannot be included in labels.
 *  - Whitespace is NOT required to separate any elements.
 *
 *  Approximate grammar:
 *
 *      EventList := Event 
 *                 | EventList Event
 *
 *      Event := '@' Time Label
 *
 *      Time := Interval
 *            | ConcreteTime
 *
 *      Interval := '[' INTEGER ',' INTEGER ']'
 *
 *      ConcreteTime := DECIMAL
 *
 *      Label := * A string that does not include whitespace or '@' *
 */

namespace monitaal {

    class EventParser {
public:
        static std::vector<concrete_input> parse_concrete_input(std::istream* stream);

        static std::vector<interval_input> parse_interval_input(std::istream* stream);
    };

}

#endif //MONITAAL_EVENT_PARSER_H
