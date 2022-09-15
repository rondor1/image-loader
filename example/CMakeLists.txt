set(sources example01.cpp
            example02.cpp)

foreach(exampleSource ${sources})
    string(REPLACE ".cpp" "" exampleName ${exampleSource})
    add_executable(${exampleName} ${exampleSource})
    target_link_libraries(${exampleName} ${PROJECT_NAME}::loader
                              ${PROJECT_NAME}::utils)
endforeach()
