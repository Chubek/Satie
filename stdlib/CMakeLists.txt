add_library(satie_stdlib INTERFACE)
add_library(satie::stdlib ALIAS satie_stdlib)
target_link_libraries(satie_stdlib INTERFACE satie::satie)
