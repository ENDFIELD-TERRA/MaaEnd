#pragma once

#include <cstdint>
#include <string>

#include <MaaFramework/MaaAPI.h>

namespace mapnavigator
{

std::string DetectControllerType(MaaController* controller);

bool TryGetWin32MouseInputMethod(MaaController* controller, MaaWin32InputMethod* out_method);
bool IsMessageInputMethod(MaaWin32InputMethod method);
bool TryGetWin32WindowHandle(MaaController* controller, uint64_t* out_handle);

} // namespace mapnavigator
