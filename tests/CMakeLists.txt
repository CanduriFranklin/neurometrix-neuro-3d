find_package(GTest CONFIG REQUIRED)
find_package(OpenMP REQUIRED)

add_executable(neuro-3d-tests
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_hounsfield.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_volume_3d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_physics_filters_3d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_morphological_ops_3d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_bilateral_symmetry_3d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_volumetric_metrics.cpp
)
target_include_directories(neuro-3d-tests PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(neuro-3d-tests PRIVATE
    GTest::gtest GTest::gtest_main Eigen3::Eigen OpenMP::OpenMP_CXX)
if(MSVC)
    target_compile_options(neuro-3d-tests PRIVATE /openmp)
endif()
include(GoogleTest)
gtest_discover_tests(neuro-3d-tests)

function(add_neuro_benchmark NAME SRC)
    add_executable(${NAME} ${CMAKE_CURRENT_SOURCE_DIR}/performance/${SRC})
    target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(${NAME} PRIVATE Eigen3::Eigen OpenMP::OpenMP_CXX)
    if(MSVC)
        target_compile_options(${NAME} PRIVATE /openmp)
    endif()
endfunction()

add_neuro_benchmark(benchmark_volume benchmark_volume.cpp)