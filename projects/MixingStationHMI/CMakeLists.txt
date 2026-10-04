cmake_minimum_required(VERSION 3.16)

project(MixingStationHMI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Quick Sql)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appMixingStationHMI
    src/main.cpp

    src/accesspolicy.cpp
    src/accesspolicy.h
    src/batchcontroller.cpp
    src/batchcontroller.h
    src/drainservice.cpp
    src/drainservice.h
    src/eventmanager.cpp
    src/eventmanager.h
    src/faultresetservice.cpp
    src/faultresetservice.h
    src/mixer.cpp
    src/mixer.h
    src/mixingtank.cpp
    src/mixingtank.h
    src/operatorsession.cpp
    src/operatorsession.h
    src/processeventcoordinator.cpp
    src/processeventcoordinator.h
    src/processsimulator.cpp
    src/processsimulator.h
    src/pump.cpp
    src/pump.h
    src/trendrecorder.cpp
    src/trendrecorder.h
    src/usermanagementservice.cpp
    src/usermanagementservice.h
    src/valve.cpp
    src/valve.h

    src/database/databaseschema.cpp
    src/database/databaseschema.h
    src/database/demouserseeder.cpp
    src/database/demouserseeder.h
    src/database/eventhistorymodel.cpp
    src/database/eventhistorymodel.h
    src/database/eventrepository.cpp
    src/database/eventrepository.h
    src/database/runlistmodel.cpp
    src/database/runlistmodel.h
    src/database/runpersistencecoordinator.cpp
    src/database/runpersistencecoordinator.h
    src/database/runrepository.cpp
    src/database/runrepository.h
    src/database/trendhistorysource.cpp
    src/database/trendhistorysource.h
    src/database/trendsamplerepository.cpp
    src/database/trendsamplerepository.h
    src/database/userlistmodel.cpp
    src/database/userlistmodel.h
    src/database/userrepository.cpp
    src/database/userrepository.h
)

qt_add_qml_module(appMixingStationHMI
    URI MixingStationHMI

    QML_FILES
        qml/AlarmBar.qml
        qml/BatchActions.qml
        qml/EquipmentLane.qml
        qml/EquipmentPanel.qml
        qml/HMIScreen.qml
        qml/PreCheckPanel.qml
        qml/ProcessValues.qml
        qml/SimulationControlPanel.qml
        qml/StageStrip.qml
        qml/TankView.qml
        qml/TrendChart.qml
        qml/TrendPanel.qml
        qml/UserManagementDialog.qml

    RESOURCES
        qml/assets/pump.png
        qml/assets/pump_gray.png
        qml/assets/valve.png
        qml/assets/valve_gray.png
)

set_target_properties(appMixingStationHMI PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.lukaszcitko.mixingstationhmi
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING
        ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appMixingStationHMI
    PRIVATE
        Qt6::Quick
        Qt6::Sql
)

install(TARGETS appMixingStationHMI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
