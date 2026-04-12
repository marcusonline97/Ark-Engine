#include "Initializer.h"
#include "ArkApp.h"
#include "Logger.h"
#include "AssetManager.h"
#include "Utility/Utility.h"
#include <memory>

namespace Init {
    static std::unique_ptr<App> s_app;

    bool StartUp() {
        Logging::EnableLevel(Logging::Level::INIT);
        Logging::EnableLevel(Logging::Level::DEBUG);
        Logging::EnableLevel(Logging::Level::_ERROR);
        Logging::EnableLevel(Logging::Level::FATAL);
        Logging::EnableLevel(Logging::Level::TODO);
        Logging::EnableLevel(Logging::Level::WARNING);
        Logging::EnableLevel(Logging::Level::FUNCTION);
        Logging::ToDo() << "Logger Init";

        try {
            Logging::ToDo() << "Initializer::StartUp";
            auto app = std::make_unique<App>();

            const std::string iconAbs = AssetManager::Instance().ResolveAssetPath("Resources/Icon/Ark.png");
            Util::SetWindowIcon(app->GetWindowHandle(), iconAbs);
            s_app = std::move(app);
        }
        catch (const std::exception& e) {
            Logging::Fatal() << "Initializer has failed: " << e.what();
            s_app.reset();
            return false;
        }
        return true;
    }

    int Run() {
        if (!s_app) return -1;
        Logging::ToDo() << "Initializer Running App";
        s_app->Run();
        return 0;
    }

    void ShutDown() {
        Logging::Init() << "Shutting Down App";
        s_app.reset();
        Logging::Shutdown();
    }
}