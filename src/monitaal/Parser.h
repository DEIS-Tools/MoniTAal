/*
 * Copyright Thomas M. Grosen 
 * Created on 20/05/2022
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

#ifndef FIXPOINT_PARSER_H
#define FIXPOINT_PARSER_H

#include "TA.h"
#include "types.h"

#include <pugixml.hpp>

namespace monitaal {

    enum operator_e {
        ASSIGN, EQ, G, GE, L, LE
    };

    class Parser {
    public:
        static TA parse(const char *path, const char *name);

    private:
        static bool load_file(pugi::xml_document& doc, const char *path);

        static constraints_t parse_constraint(const std::string& input, std::vector<std::string>& clocks);

        static clocks_t parse_reset(const std::string& input, std::vector<std::string>& clocks);

        static location_id_t parse_loc_id(const char *input);

        static clock_index_t parse_clock(const std::string& clock, std::vector<std::string>& clocks);

        static bool clock_exists(const std::string& clock, const std::vector<std::string>& clocks);

        static clock_index_t get_index(const std::string& clock, const std::vector<std::string>& clocks);

        static operator_e parse_operator(const std::string& op);
    };

}

#endif //FIXPOINT_PARSER_H
