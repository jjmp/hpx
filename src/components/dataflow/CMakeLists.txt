# Copyright (c) 2007-2012 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(root "${hpx_SOURCE_DIR}/hpx/components/dataflow")

add_hpx_component(dataflow
  FOLDER "Core/Components"
  HEADER_ROOT ${root}
  AUTOGLOB)

add_hpx_pseudo_dependencies(components.dataflow
                            dataflow_component)

