cmake_minimum_required(VERSION 3.21)
project(SystemResourceMonitor VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)
find_package(GTest REQUIRED)
find_package(SQLite3 REQUIRED)

add_executable(system-resource-monitor
    src/main.cpp
    src/core/FileUtil.cpp
    src/core/MetricMath.cpp
    src/core/MetricHistory.cpp
    src/core/DataCollector.cpp
    src/core/SystemMonitor.cpp
    src/monitors/CpuMonitor.cpp
    src/monitors/MemoryMonitor.cpp
    src/monitors/DiskMonitor.cpp
    src/monitors/NetworkMonitor.cpp
    src/monitors/TemperatureMonitor.cpp
    src/monitors/BatteryMonitor.cpp
    src/database/DatabaseRepository.cpp
    src/database/DatabaseManager.cpp
    src/hardware/HardwareService.cpp
    src/process/ProcessInfo.cpp
    src/process/ProcessService.cpp
    src/process/ProcessWorker.cpp
    src/services/ConfigManager.cpp
    src/services/HistoryService.cpp
    src/services/ApplicationServices.cpp
    src/ui/MainWindow.cpp
    src/ui/Dashboard.cpp
    src/ui/MonitoringPage.cpp
    src/ui/MetricCard.cpp
    src/ui/ChartUtils.cpp
    src/ui/FormatUtils.cpp
    src/ui/SettingsPage.cpp
    src/ui/PlaceholderPage.cpp
    src/ui/HardwarePage.cpp
    src/ui/ProcessPage.cpp
    src/ui/AnalyticsPage.cpp
    include/core/Metrics.h
    include/core/FileUtil.h
    include/core/MetricMath.h
    include/core/MetricHistory.h
    include/core/DataCollector.h
    include/core/SystemMonitor.h
    include/monitors/CpuMonitor.h
    include/monitors/MemoryMonitor.h
    include/monitors/DiskMonitor.h
    include/monitors/NetworkMonitor.h
    include/monitors/TemperatureMonitor.h
    include/monitors/BatteryMonitor.h
    include/database/DatabaseTypes.h
    include/database/DatabaseRepository.h
    include/database/DatabaseManager.h
    include/hardware/HardwareInfo.h
    include/hardware/HardwareService.h
    include/process/ProcessInfo.h
    include/process/ProcessService.h
    include/process/ProcessWorker.h
    include/services/ConfigManager.h
    include/services/HistoryService.h
    include/services/ApplicationServices.h
    include/ui/MainWindow.h
    include/ui/Dashboard.h
    include/ui/MonitoringPage.h
    include/ui/MetricCard.h
    include/ui/ChartUtils.h
    include/ui/FormatUtils.h
    include/ui/SettingsPage.h
    include/ui/PlaceholderPage.h
    include/ui/HardwarePage.h
    include/ui/ProcessPage.h
    include/ui/AnalyticsPage.h
)

target_include_directories(system-resource-monitor PRIVATE include)
target_link_libraries(system-resource-monitor PRIVATE Qt6::Widgets Qt6::Charts SQLite3::SQLite3)
target_compile_options(system-resource-monitor PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()

add_executable(unit_tests
    tests/test_parsers.cpp
    tests/test_hardware.cpp
    tests/test_process.cpp
    tests/test_database.cpp
    tests/test_sensors.cpp
    src/core/FileUtil.cpp
    src/core/MetricMath.cpp
    src/core/MetricHistory.cpp
    src/monitors/CpuMonitor.cpp
    src/monitors/MemoryMonitor.cpp
    src/monitors/DiskMonitor.cpp
    src/monitors/NetworkMonitor.cpp
    src/monitors/TemperatureMonitor.cpp
    src/monitors/BatteryMonitor.cpp
    src/hardware/HardwareService.cpp
    src/process/ProcessInfo.cpp
    src/database/DatabaseRepository.cpp
)

target_include_directories(unit_tests PRIVATE include)
target_link_libraries(unit_tests PRIVATE GTest::gtest_main Qt6::Core SQLite3::SQLite3)
target_compile_options(unit_tests PRIVATE -Wall -Wextra -Wpedantic)

add_test(NAME unit_tests COMMAND unit_tests)
