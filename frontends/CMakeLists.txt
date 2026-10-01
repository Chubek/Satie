add_library(satie_frontends INTERFACE)
add_library(satie::frontends ALIAS satie_frontends)
target_link_libraries(satie_frontends INTERFACE satie::satie)
