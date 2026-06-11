if                          (NOT APPLE)
   set                      (linker_flags "-pie -Wl,--export-dynamic")
   set                      (linker_flags "${linker_flags} -Wl,-u,henson_set_contexts,-u,henson_set_namemap")
else                        ()
   set                      (linker_flags "-Wl,-u,_henson_set_contexts,-u,_henson_set_namemap")
endif                       (NOT APPLE)

add_library                 (consumer SHARED consumer.cpp)
target_link_libraries       (consumer ${libraries})
set_target_properties       (consumer PROPERTIES PREFIX "")
set_target_properties       (consumer PROPERTIES SUFFIX ".so")
set_target_properties       (consumer PROPERTIES LINK_FLAGS ${linker_flags})

add_library                 (producer-hdf5 SHARED producer-hdf5.cpp)
target_link_libraries       (producer-hdf5 ${libraries})
set_target_properties       (producer-hdf5 PROPERTIES PREFIX "")
set_target_properties       (producer-hdf5 PROPERTIES SUFFIX ".so")
set_target_properties       (producer-hdf5 PROPERTIES LINK_FLAGS ${linker_flags})

add_library                 (consumer-hdf5 SHARED consumer-hdf5.cpp)
target_link_libraries       (consumer-hdf5 ${libraries})
set_target_properties       (consumer-hdf5 PROPERTIES PREFIX "")
set_target_properties       (consumer-hdf5 PROPERTIES SUFFIX ".so")
set_target_properties       (consumer-hdf5 PROPERTIES LINK_FLAGS ${linker_flags})

add_library                 (producer-netcdf SHARED producer-netcdf.cpp)
target_link_libraries       (producer-netcdf ${libraries})
set_target_properties       (producer-netcdf PROPERTIES PREFIX "")
set_target_properties       (producer-netcdf PROPERTIES SUFFIX ".so")
set_target_properties       (producer-netcdf PROPERTIES LINK_FLAGS ${linker_flags})

add_library                 (consumer-netcdf SHARED consumer-netcdf.cpp)
target_link_libraries       (consumer-netcdf ${libraries})
set_target_properties       (consumer-netcdf PROPERTIES PREFIX "")
set_target_properties       (consumer-netcdf PROPERTIES SUFFIX ".so")
set_target_properties       (consumer-netcdf PROPERTIES LINK_FLAGS ${linker_flags})

install                     (TARGETS
                            consumer
                            producer-hdf5
                            consumer-hdf5
                            producer-netcdf
                            consumer-netcdf
                            DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
                            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ WORLD_WRITE WORLD_EXECUTE)
