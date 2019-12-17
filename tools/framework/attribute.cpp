#include "attribute.hpp"

namespace tai {

    bool cmp(const S_Attribute lhs, const S_Attribute rhs) {
        return lhs->cmp(rhs);
    }

    inline bool operator==(const S_Attribute lhs, const S_Attribute rhs) { return cmp(lhs, rhs) == 0; }
    inline bool operator!=(const S_Attribute lhs, const S_Attribute rhs) { return cmp(lhs, rhs) != 0; }
    std::ostream& operator<<(std::ostream& os, const S_Attribute attr) { return attr->str(os); }

}
