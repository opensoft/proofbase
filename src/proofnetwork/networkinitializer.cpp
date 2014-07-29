#include "abstractrestapi.h"

__attribute__((constructor))
void libraryInit()
{
    qRegisterMetaType<Proof::RestApiError>("Proof::RestApiError");
}
