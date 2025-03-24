#include "monitaal/TA.h"

using namespace monitaal;

std::pair<TA, TA> gear_control_newgear_prop() {
    location_id_t init = 0, g0 = 1, g1 = 2, g2 = 3, n0 = 4, n1 = 5, n2 = 6, out = 7;

    std::string req_label = "ReqNewGear",
                new_gear = "NewGear",
                usecase1 = "UseCase1",
                usecase2 = "UseCase2";

    std::vector<std::string> non_neutrals = {
        req_label + "23",
        req_label + "32",
        req_label + "34",
        req_label + "43",
        req_label + "45",
        req_label + "54",
        req_label + "56",
        req_label + "65",
        req_label + "67",
        req_label + "76"};

    std::vector<std::string> neutrals = {
        req_label + "01",
        req_label + "10",
        req_label + "12",
        req_label + "21"};

    edges_t edges;

    for (const auto& s : non_neutrals) {
        edges.push_back({init, g0, {}, {1}, s});
        edges.push_back({g0, out, {}, {}, s});
        edges.push_back({g1, out, {}, {}, s});
        edges.push_back({g2, out, {}, {}, s});
        edges.push_back({n0, out, {}, {}, s});
        edges.push_back({n1, out, {}, {}, s});
        edges.push_back({n2, out, {}, {}, s});
        edges.push_back({out, out, {}, {}, s});
    }

    for (const auto& s : neutrals) {
        edges.push_back({init, n0, {}, {1}, s});
        edges.push_back({g0, out, {}, {}, s});
        edges.push_back({g1, out, {}, {}, s});
        edges.push_back({g2, out, {}, {}, s});
        edges.push_back({n0, out, {}, {}, s});
        edges.push_back({n1, out, {}, {}, s});
        edges.push_back({n2, out, {}, {}, s});
        edges.push_back({out, out, {}, {}, s});
    }

    edges.push_back({init, out, {}, {}, new_gear});
    edges.push_back({g0, init, {constraint_t::upper_non_strict(1, 900), constraint_t::lower_non_strict(1, 400)}, {}, new_gear});
    edges.push_back({g1, init, {constraint_t::lower_non_strict(1, 700), constraint_t::upper_strict(1, 1055)}, {}, new_gear});
    edges.push_back({g2, init, {constraint_t::lower_non_strict(1, 750), constraint_t::upper_non_strict(1, 1205)}, {}, new_gear});
    edges.push_back({n0, init, {constraint_t::lower_non_strict(1, 150), constraint_t::upper_non_strict(1, 900)}, {}, new_gear});
    edges.push_back({n1, init, {constraint_t::lower_non_strict(1, 550), constraint_t::upper_strict(1, 1055)}, {}, new_gear});
    edges.push_back({n2, init, {constraint_t::lower_non_strict(1, 450), constraint_t::upper_non_strict(1, 1205)}, {}, new_gear});


    edges.push_back({g0, out, {constraint_t::upper_strict(1, 400)}, {}, new_gear});
    edges.push_back({g0, out, {constraint_t::lower_strict(1, 900)}, {}, new_gear});

    edges.push_back({g1, out, {constraint_t::upper_strict(1, 700)}, {}, new_gear});
    edges.push_back({g1, out, {constraint_t::lower_non_strict(1, 1055)}, {}, new_gear});

    edges.push_back({g2, out, {constraint_t::upper_strict(1, 750)}, {}, new_gear});
    edges.push_back({g2, out, {constraint_t::lower_strict(1, 1205)}, {}, new_gear});



    edges.push_back({n0, out, {constraint_t::upper_strict(1, 150)}, {}, new_gear});
    edges.push_back({n0, out, {constraint_t::lower_strict(1, 900)}, {}, new_gear});

    edges.push_back({n1, out, {constraint_t::upper_strict(1, 550)}, {}, new_gear});
    edges.push_back({n1, out, {constraint_t::lower_non_strict(1, 1055)}, {}, new_gear});

    edges.push_back({n2, out, {constraint_t::upper_strict(1, 450)}, {}, new_gear});
    edges.push_back({n2, out, {constraint_t::lower_strict(1, 1205)}, {}, new_gear});


    edges.push_back({g0, g1, {constraint_t::upper_non_strict(1, 900)}, {}, usecase1});
    edges.push_back({g0, g2, {constraint_t::upper_non_strict(1, 900)}, {}, usecase2});
    edges.push_back({n0, n1, {constraint_t::upper_non_strict(1, 900)}, {}, usecase1});
    edges.push_back({n0, n2, {constraint_t::upper_non_strict(1, 900)}, {}, usecase2});


    edges.push_back({g0, out, {constraint_t::lower_strict(1, 900)}, {}, usecase1});
    edges.push_back({g0, out, {constraint_t::lower_strict(1, 900)}, {}, usecase2});
    edges.push_back({n0, out, {constraint_t::lower_strict(1, 900)}, {}, usecase1});
    edges.push_back({n0, out, {constraint_t::lower_strict(1, 900)}, {}, usecase2});
    
    
    
    edges.push_back({init, out, {}, {}, usecase1});
    edges.push_back({init, out, {}, {}, usecase2});
    
    edges.push_back({g1, out, {}, {}, usecase1});
    edges.push_back({g2, out, {}, {}, usecase1});
    edges.push_back({g1, out, {}, {}, usecase2});
    edges.push_back({g2, out, {}, {}, usecase2});

    edges.push_back({n1, out, {}, {}, usecase1});
    edges.push_back({n2, out, {}, {}, usecase1});
    edges.push_back({n1, out, {}, {}, usecase2});
    edges.push_back({n2, out, {}, {}, usecase2});


    edges.push_back({out, out, {}, {}, usecase1});
    edges.push_back({out, out, {}, {}, usecase2});
    edges.push_back({out, out, {}, {}, new_gear});


    locations_t pos_loc{
        location_t(true, init, "init", {}),
        location_t(false, g0, "g0", {}),
        location_t(false, g1, "g1", {}),
        location_t(false, g2, "g2", {}),
        location_t(false, n0, "n0", {}),
        location_t(false, n1, "n1", {}),
        location_t(false, n2, "n2", {}),
        location_t(false, out, "out", {})};

    locations_t neg_loc{
        location_t(false, init, "init", {}),
        location_t(false, g0, "g0", {}),
        location_t(false, g1, "g1", {}),
        location_t(false, g2, "g2", {}),
        location_t(false, n0, "n0", {}),
        location_t(false, n1, "n1", {}),
        location_t(false, n2, "n2", {}),
        location_t(true, out, "out", {})};
    
    return {TA("positive", {{0, "0"}, {1, "x"}}, pos_loc, edges, init), TA("negative", {{0, "0"}, {1, "x"}}, neg_loc, edges, init)};
}