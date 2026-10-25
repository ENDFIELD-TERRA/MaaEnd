#include <utility>

#include <MaaUtils/Logger.h>

#include "../../MapLocator/MapLocateAction.h"
#include "../control/controller_info_utils.h"
#include "../control/controller_type_utils.h"
#include "Adb/adb_input_backend.h"
#include "Desktop/desktop_input_backend.h"
#include "Linux/linux_input_backend.h"
#include "backend.h"

namespace mapnavigator
{

std::unique_ptr<IInputBackend> CreateInputBackend(MaaController* ctrl)
{
    std::string controller_type = DetectControllerType(ctrl);
    if (controller_type.empty()) {
        controller_type = "unknown";
    }

    if (IsAdbLikeControllerType(controller_type)) {
        return backend::adb::CreateAdbInputBackend(ctrl, std::move(controller_type), maplocator::getOrInitLocator());
    }

    if (IsLinuxControllerType(controller_type)) {
        return backend::Linux::CreateLinuxInputBackend(ctrl, std::move(controller_type));
    }

    if (IsCloudWebControllerType(controller_type)) {
        // CloudWeb 复用桌面输入原语（generic PostClick/PostKeyDown/PostRelativeMove），
        // 但不得挂 Win32 专属的 mouse lock / background managed keys，后端名也不得记成 win32。
        return backend::desktop::CreateDesktopInputBackend(ctrl, std::move(controller_type), "cloud_web", false);
    }

    return backend::desktop::CreateDesktopInputBackend(ctrl, std::move(controller_type), "win32");
}

} // namespace mapnavigator
