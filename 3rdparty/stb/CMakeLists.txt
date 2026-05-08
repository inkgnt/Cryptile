
add_library(stb_truetype STATIC stb_truetype.h)
add_library(stb_truetype_interface INTERFACE)

target_include_directories(stb_truetype_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(stb_truetype PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(stb_truetype PRIVATE STB_TRUETYPE_IMPLEMENTATION)


set_target_properties(stb_truetype PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

set_source_files_properties(stb_truetype.h PROPERTIES LANGUAGE C)