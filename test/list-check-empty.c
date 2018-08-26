#include <assert.h>

#include "list.h"

int main()
{
    struct list uut;

    list_init(&uut);

    assert(list_isempty(&uut));

    return 0;
}
