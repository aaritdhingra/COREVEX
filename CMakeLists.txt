cmake_minimum_required(VERSION 3.25)
project(COREVEX VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Sql WebEngineCore WebEngineWidgets)

# ============================================================================
# SECTION — BUILD (modular sources, ek giant file nahi)
# ----------------------------------------------------------------------------
# Qt version: laptop par Qt 6.7.3 (C:\Qt\6.7.3\msvc2019_64) hai, isliye
# koi extra install nahi chahiye. CMAKE_PREFIX_PATH wahi do (build_core.bat).
# Har naya module yahan add karo, warna wo build hi nahi hoga.
# ============================================================================
add_executable(COREVEX
    assets/corevex.qrc
    src/app/main.cpp
    src/app/CorevexWindow.cpp
    src/utilities/Assets.cpp
    src/utilities/UrlUtils.cpp
    src/utilities/ThemeInjector.cpp
    src/ui/NewTabPage.cpp
    src/ui/Pages.cpp
    src/ui/AdsLabPage.cpp
    src/ui/AiSettingsDialog.cpp
    src/ui/Animations.cpp
    src/search/SearchEngine.cpp
    src/search/CustomSearch.cpp
    src/storage/Store.cpp
    src/ai/AiClient.cpp
)
target_include_directories(COREVEX PRIVATE src)

target_link_libraries(COREVEX PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Sql
    Qt6::WebEngineCore Qt6::WebEngineWidgets
)
if(MSVC)
    target_compile_options(COREVEX PRIVATE /utf-8)
endif()