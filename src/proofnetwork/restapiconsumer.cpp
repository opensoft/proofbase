#include "restapiconsumer.h"

using namespace Proof;

RestApiConsumer::ErrorCallbackType RestApiConsumer::generateErrorCallback(qulonglong &currentOperationId,
                                                                          QString &errorMessage) const
{
    return Proof::AbstractRestApi::generateErrorCallback(currentOperationId, errorMessage);
}

RestApiConsumer::ErrorCallbackType RestApiConsumer::generateErrorCallback(qulonglong &currentOperationId,
                                                                          Proof::RestApiError &error) const
{
    return Proof::AbstractRestApi::generateErrorCallback(currentOperationId, error);
}
