//  Copyright (c) 2011 Thomas Heller
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !BOOST_PP_IS_ITERATING

#ifndef HPX_FUNCTION_DETAIL_GET_TABLE_HPP
#define HPX_FUNCTION_DETAIL_GET_TABLE_HPP

#include <boost/detail/sp_typeinfo.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>

#define BOOST_PP_ITERATION_PARAMS_1                                             \
    (                                                                           \
        3                                                                       \
      , (                                                                       \
            0                                                                   \
          , HPX_FUNCTION_LIMIT                                                  \
          , <hpx/util/detail/get_table.hpp>                                     \
        )                                                                       \
    )                                                                           \
/**/
#include BOOST_PP_ITERATE()

#endif

#else

#define N BOOST_PP_ITERATION()

namespace hpx { namespace util { namespace detail {

    template <
        typename Functor
      , typename R
      BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, typename A)
    >
    struct get_table<
        Functor
      , R(BOOST_PP_ENUM_PARAMS(N, A))
    >
    {
        template <typename IArchive, typename OArchive>
        static vtable_ptr_base<
            R(BOOST_PP_ENUM_PARAMS(N, A))
          , IArchive
          , OArchive
        >*
        get()
        {
            typedef
                typename vtable<sizeof(Functor) <= sizeof(void *)>::
                    template type<
                        Functor
                      , R(BOOST_PP_ENUM_PARAMS(N, A))
                      , IArchive
                      , OArchive
                    >
                vtable_type;

            typedef
                vtable_ptr<
                    R(BOOST_PP_ENUM_PARAMS(N, A))
                  , IArchive
                  , OArchive
                  , vtable_type
                >
                vtable_ptr_type;

            static vtable_ptr_type ptr;

            return &ptr;
        }
    };
}}}

#undef N

#endif
