/*
 * Copyright Thomas M. Grosen 
 * Created on 11/01/2022
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

#include "EventParser.h"
#include "TA.h"
#include "errors.h"

#include <stdio.h>
#include <string>
#include <istream>
#include <iostream>
#include <locale>

namespace monitaal {

    void skip_space(std::istream* stream) {
        char c = stream->peek();
        while(c == ' ' || c == '\t') {
            stream->get(); 
            c = stream->peek();
        }
        return;
    }

    void read_seperator(std::istream* stream) {
        char c;
        skip_space(stream);
        if (not (*stream >> c && c == '@'))
            throw base_error("Expected a @ separator at ", stream->tellg(), " but got \"", c, "\"");
        return;
    }

    template<bool is_interval> typename std::conditional_t<is_interval, interval_t, long double> 
    read_time(std::istream* stream);

    template<> long double read_time<false>(std::istream* stream) {
        long double time;
        *stream >> time;
        return time;
    }

    template<> interval_t read_time<true>(std::istream* stream) {
        int lower, upper;
        char c;

        if ( *stream >> std::ws >> c && c == '[' &&
            *stream >> lower &&
            *stream >> std::ws >> c && c == ',' &&
            *stream >> upper &&
            *stream >> std::ws >> c && c == ']')
            return {lower, upper};
        else {
            throw base_error("Error parsing interval input at ", stream->tellg(), " but got \"", stream->peek(), "\"");
        }
    }

    std::string read_observation(std::istream* stream) {
        std::string observation = "";
        char c;
        
        skip_space(stream);

        c = stream->peek();
        while (c != '\n' && c != '@' && !stream->eof() && c != '\000' && c != '\t' && c != ' ') {
            observation += stream->get();
            c = stream->peek();
        }

        return observation;
    }

    template<bool is_interval> std::vector<typename std::conditional_t<is_interval, interval_input, concrete_input>> 
    EventParser::parse_input(std::istream* stream, uint32_t limit) {
        std::string observation;
        typename std::conditional_t<is_interval, interval_t, long double> time;

        std::vector<typename std::conditional_t<is_interval, interval_input, concrete_input>> events;

        uint32_t counter = 0;

        *stream >> std::ws;
        while (not stream->eof() && stream->peek() != '\000' && (limit == 0 || counter < limit)) {
            read_seperator(stream);
            time = read_time<is_interval>(stream);

            observation = read_observation(stream);

            events.emplace_back(time, observation);
            ++counter;

            *stream >> std::ws;
        }

        return events;
    }

    template std::vector<interval_input> EventParser::parse_input<true>(std::istream* stream, uint32_t limit);
    template std::vector<concrete_input> EventParser::parse_input<false>(std::istream* stream, uint32_t limit);

}