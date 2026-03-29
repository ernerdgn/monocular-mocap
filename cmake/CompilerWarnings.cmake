add_library(mocap_warnings INTERFACE)

if(MSVC)
    target_compile_options(mocap_warnings INTERFACE
        /W4 /WX /permissive- /w14242 /w14254 /w14263 /w14265 
        /w14287 /we4289 /w14296 /w14311 /w14545 /w14546 
        /w14547 /w14549 /w14555 /w14619 /w14640 /w14826 
        /w14905 /w14906 /w14928 /wd4251
    )
else()
    target_compile_options(mocap_warnings INTERFACE
        -Wall -Wextra -Wpedantic -Werror -Wshadow -Wnon-virtual-dtor 
        -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual 
        -Wconversion -Wsign-conversion -Wnull-dereference 
        -Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough
    )
endif()