cmake_minimum_required(VERSION 3.20)
project(PivotHarvest VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# nlohmann/json — grab via vcpkg or FetchContent
find_package(nlohmann_json 3.11 REQUIRED)

add_executable(PivotHarvest
    main.cpp
    HttpClient.cpp
    InputDetector.cpp
    ResultAggregator.cpp
    modules/IPModule.cpp
    modules/EmailModule.cpp
    modules/PhoneModule.cpp
    modules/NameModule.cpp
)

target_link_libraries(PivotHarvest
    PRIVATE
    nlohmann_json::nlohmann_json
    winhttp
    ws2_32
)

target_compile_options(PivotHarvest PRIVATE /W4 /WX)
