#pragma once

#include <string>

namespace sevun {

    class hex_formatter {
    public:
        static std::string dump_to_string(
            const void* data,
            size_t size);
    };

};
