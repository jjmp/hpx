//  Copyright (c) 2007-2012 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPX_PARTITION3D_AUG_06_2011_1020PM)
#define HPX_PARTITION3D_AUG_06_2011_1020PM

#include <hpx/hpx_fwd.hpp>
#include <hpx/lcos/future.hpp>
#include <hpx/runtime/components/client_base.hpp>

#include "stubs/partition3d.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace interpolate3d
{
    class partition3d
      : public hpx::components::client_base<
            partition3d, interpolate3d::stubs::partition3d>
    {
    private:
        typedef hpx::components::client_base<
            partition3d, interpolate3d::stubs::partition3d> base_type;

    public:
        // create a new partition instance and initialize it synchronously
        partition3d(std::string const& datafilename, dimension const& dimx,
                dimension const& dimy, dimension const& dimz)
          : base_type(interpolate3d::stubs::partition3d::create_sync(hpx::find_here()))
        {
            init(datafilename, dimx, dimy, dimz);
        }
        partition3d(hpx::naming::id_type gid, std::string const& datafilename,
                dimension const& dimx, dimension const& dimy, dimension const& dimz)
          : base_type(interpolate3d::stubs::partition3d::create_sync(gid))
        {
            init(datafilename, dimx, dimy, dimz);
        }
        partition3d(hpx::naming::id_type gid)
          : base_type(gid)
        {}

        // initialize this partition
        hpx::lcos::future<void>
        init_async(std::string const& datafilename,
            dimension const& dimx, dimension const& dimy, dimension const& dimz)
        {
            return stubs::partition3d::init_async(this->gid_, datafilename,
                dimx, dimy, dimz);
        }

        void init(std::string const& datafilename,
            dimension const& dimx, dimension const& dimy, dimension const& dimz)
        {
            stubs::partition3d::init(this->gid_, datafilename, dimx, dimy, dimz);
        }

        // ask this partition to interpolate, note that value must be in the
        // range valid for this partition
        hpx::lcos::future<double>
        interpolate_async(double value_x, double value_y, double value_z)
        {
            return stubs::partition3d::interpolate_async(this->gid_,
                value_x, value_y, value_z);
        }

        double interpolate(double value_x, double value_y, double value_z)
        {
            return stubs::partition3d::interpolate(this->gid_,
                value_x, value_y, value_z);
        }
    };
}

#endif


