#include "abstractrestapi.h"

__attribute__((constructor))
static void libraryInit()
{
    qRegisterMetaType<Proof::RestApiError>("Proof::RestApiError");
}
