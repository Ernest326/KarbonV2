file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Exclude the runtime entry point from the engine library
list(REMOVE_ITEM ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

add_library(karbon_engine STATIC ${ENGINE_SOURCES})

target_compile_definitions(karbon_engine PUBLIC GLFW_INCLUDE_NONE=1)

if(MSVC)
    target_compile_definitions(karbon_engine PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

# Production vs Development defines
if(PRODUCTION_BUILD)
    target_compile_definitions(karbon_engine PUBLIC
        RESOURCES_PATH="./resources"
        PRODUCTION_BUILD=1
        DEVELOPMENT_BUILD=0
    )
else()
    target_compile_definitions(karbon_engine PUBLIC
        RESOURCES_PATH="${CMAKE_SOURCE_DIR}/resources"
        PRODUCTION_BUILD=0
        DEVELOPMENT_BUILD=1
    )
endif()

target_include_directories(karbon_engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/thirdparty/glfw/include"
    "${CMAKE_SOURCE_DIR}/thirdparty/glad/include"
    "${CMAKE_SOURCE_DIR}/thirdparty/glm"
    "${CMAKE_SOURCE_DIR}/thirdparty/stb_image"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui"
    "${CMAKE_SOURCE_DIR}/thirdparty/jolt/JoltPhysics/Build"
    "${CMAKE_SOURCE_DIR}/thirdparty/entt/single_include"
    "${CMAKE_SOURCE_DIR}/thirdparty/assimp/include"
)

target_link_libraries(karbon_engine PUBLIC
    glm
    glfw
    glad
    stb_image
    imgui
    jolt
    assimp
    EnTT::EnTT
)

# Runtime executable
add_executable(Karbon "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
set_property(TARGET Karbon PROPERTY CXX_STANDARD 17)
target_link_libraries(Karbon PRIVATE karbon_engine)

if(MSVC)
    set_target_properties(Karbon PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()