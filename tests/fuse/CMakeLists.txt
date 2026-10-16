if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    link_directories(/usr/local/lib/x86_64-linux-gnu/ /usr/lib64 /usr/local/lib64)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    link_directories(/usr/local/lib/aarch64-linux-gnu/ /usr/lib64 /usr/local/lib64)
endif()

target_add_test(test_fuse hf3fs_fuse)
