file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(karbon_editor ${EDITOR_SOURCES})

set_property(TARGET karbon_editor PROPERTY CXX_STANDARD 17)

if(MSVC)
    target_compile_definitions(karbon_editor PUBLIC _CRT_SECURE_NO_WARNINGS)
    set_target_properties(karbon_editor PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

target_include_directories(karbon_editor PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/src/engine"
)

target_link_libraries(karbon_editor PRIVATE karbon_engine imguizmo)

# Editor-specific: ensure resources path points to project root
if(NOT PRODUCTION_BUILD)
    target_compile_definitions(karbon_editor PRIVATE
        RESOURCES_PATH="${CMAKE_SOURCE_DIR}/resources"
    )
endif()