# Copyright (c) 2011 Bryce Adelstein-Lelbach
# Copyright (c) 2012 Daniel Kogler
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(benchmarks
    apply_scheduling_analysis
    general_apply_analysis
    general_pa_analysis
    get_gid_analysis
   )

set(apply_scheduling_analysis_FLAGS DEPENDENCIES iostreams_component)
set(check_address_analysis_FLAGS DEPENDENCIES iostreams_component)
set(general_apply_analysis_FLAGS DEPENDENCIES iostreams_component)
set(general_pa_analysis_FLAGS DEPENDENCIES iostreams_component)
set(get_gid_analysis_FLAGS DEPENDENCIES iostreams_component)

foreach(benchmark ${benchmarks})
  set(sources
      ${benchmark}.cpp)

  source_group("Source Files" FILES ${sources})

  # add example executable
  add_hpx_executable(${benchmark}
                     SOURCES ${sources}
                     ${${benchmark}_FLAGS}
                     FOLDER "Benchmarks/${benchmark}")

  # add a custom target for this example
  add_hpx_pseudo_target(tests.performance.${benchmark})

  # make pseudo-targets depend on master pseudo-target
  add_hpx_pseudo_dependencies(tests.performance
                              tests.performance.${benchmark})

  # add dependencies to pseudo-target
  add_hpx_pseudo_dependencies(tests.performance.${benchmark}
                              ${benchmark}_exe)
endforeach()
