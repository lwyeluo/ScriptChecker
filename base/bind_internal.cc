#include "bind_internal.h"

namespace base {
    namespace internal {

        Switcher::Switcher(){}

        Switcher::~Switcher() {
           size_t len = getArgsNum();
           for(size_t i = 0; i < len; i ++)
             args.pop_back();
         }

        int Switcher::doSwitch() {
            printf("You are in Switcher. Num of args: %zu\n", this->getArgsNum());
            return 0;
        }

    }
}
