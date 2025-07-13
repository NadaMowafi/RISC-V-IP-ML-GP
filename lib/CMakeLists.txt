add_library(lib STATIC  
            src/Gaussian_Vector.cpp
            src/BoxFilter_Vector_Template.cpp
            src/Flipping_Vector.cpp
            src/Rotate_Vector.cpp
            src/AddRVV.cpp
            #please add any other source files you need to compile here
            # e.g. src/other_file.cpp
            )

target_include_directories(lib
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)