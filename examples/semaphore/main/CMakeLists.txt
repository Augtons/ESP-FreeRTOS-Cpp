file(GLOB_RECURSE CPP_SRCS  "*.cpp")
file(GLOB_RECURSE C_SRCS    "*.c")

idf_component_register(
    SRCS            ${CPP_SRCS} ${C_SRCS}
    INCLUDE_DIRS    "."
)

foreach (cpp IN LISTS CPP_SRCS)
    set_source_files_properties(${cpp} PROPERTIES COMPILE_FLAGS "-std=gnu++17")
endforeach ()
