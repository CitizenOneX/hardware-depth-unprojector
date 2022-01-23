cmake_minimum_required(VERSION 3.0)

project(
    hdu
)

add_library(hdu hdu.c)
target_link_libraries(hdu)
install(TARGETS hdu DESTINATION lib)
install(FILES hdu.h DESTINATION include)

#add_executable(hvd-decoding-example examples/hvd_decoding_example.c)
#target_link_libraries(hvd-decoding-example hvd)


