#ifndef __BINARIES__SECTION_H_
#define __BINARIES__SECTION_H_ 1

#include <stdint.h>

typedef int64_t index_t;

namespace Binaries
{

    /**
     * @brief The section type
     */
    enum class SectionType : uint8_t
    {
        Invalid,
        Field,
        Variable,
        Function,
    };

    /**
     * @brief A section in the binary that contains variable/field data or function code
     */
    struct Section
    {
        SectionType Type;
        index_t ID;
        uint64_t DataSize;
        uint8_t* Data;
    };
}

#endif
