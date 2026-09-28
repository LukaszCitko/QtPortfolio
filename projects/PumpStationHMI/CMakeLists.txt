cmake_minimum_required(VERSION 3.16)

project(PumpStationHMI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Sql)

qt_standard_project_setup(REQUIRES 6.8)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appPumpStationHMI
    src/main.cpp
    src/pump.cpp
    src/processsimulator.cpp
    src/valve.cpp
    src/eventmanager.cpp
    src/mixingtank.cpp
    src/batchcontroller.cpp
    src/mixer.cpp
    src/database/databaseschema.h
    src/database/databaseschema.cpp
    src/database/userlistmodel.h
    src/database/userlistmodel.cpp
    src/database/runrepository.h
    src/database/runrepository.cpp
    src/processeventcoordinator.h
    src/processeventcoordinator.cpp
    src/database/runpersistencecoordinator.h
    src/database/runpersistencecoordinator.cpp
    src/database/eventrepository.h
    src/database/eventrepository.cpp
    src/database/eventhistorymodel.h
    src/database/eventhistorymodel.cpp
    src/database/trendsamplerepository.h
    src/database/trendsamplerepository.cpp
    src/database/runlistmodel.h
    src/database/runlistmodel.cpp
    src/database/trendhistorysource.h
    src/database/trendhistorysource.cpp
    src/accesspolicy.h
    src/accesspolicy.cpp
    src/faultresetservice.h
    src/faultresetservice.cpp
    src/drainservice.h
    src/drainservice.cpp
    src/database/userrepository.h
    src/database/userrepository.cpp
    src/usermanagementservice.h
    src/usermanagementservice.cpp
    src/database/demouserseeder.h
    src/database/demouserseeder.cpp





)

qt_add_qml_module(appPumpStationHMI
    URI PumpStationHMI

    QML_FILES

        qml/SimulationControlPanel.qml
        qml/HMIScreen.qml
        qml/StageStrip.qml
        qml/ProcessValues.qml
        qml/TankView.qml
        qml/PreCheckPanel.qml
        qml/EquipmentLane.qml
        qml/AlarmBar.qml
        qml/BatchActions.qml
        qml/EquipmentPanel.qml
        qml/TrendChart.qml
        qml/TrendPanel.qml
        qml/UserManagementDialog.qml

    RESOURCES
        qml/assets/pump.png
        qml/assets/valve.png
        qml/assets/pump_gray.png
        qml/assets/valve_gray.png


    SOURCES
        src/pump.h
        src/pump.cpp
        src/processsimulator.h
        src/processsimulator.cpp
        src/valve.h
        src/valve.cpp
        src/eventmanager.h
        src/eventmanager.cpp
        src/mixingtank.h
        src/mixingtank.cpp
        src/batchcontroller.h
        src/batchcontroller.cpp
        src/mixer.h
        src/mixer.cpp
        src/operatorsession.h
        src/operatorsession.cpp
        src/trendrecorder.h
        src/trendrecorder.cpp














)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appPumpStationHMI PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appPumpStationHMI
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appPumpStationHMI
    PRIVATE Qt6::Quick Qt6::Sql
)

install(TARGETS appPumpStationHMI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
