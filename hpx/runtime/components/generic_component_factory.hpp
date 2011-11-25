//  Copyright (c) 2007-2011 Hartmut Kaiser
//  Copyright (c) 2011      Bryce Lelbach
//  Copyright (c) 2011      Thomas Heller
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPX_GENERIC_COMPONENT_FACTORY_NOV_23_2010)
#define HPX_GENERIC_COMPONENT_FACTORY_NOV_23_2010

#include <hpx/config.hpp>
#include <hpx/hpx_fwd.hpp>

#include <hpx/runtime/components/unique_component_name.hpp>
#include <hpx/runtime/components/component_factory_base.hpp>
#include <hpx/runtime/components/component_registry.hpp>
#include <hpx/runtime/components/server/manage_component.hpp>
#include <hpx/util/ini.hpp>

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/detail/atomic_count.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace hpx { namespace components
{
    ///////////////////////////////////////////////////////////////////////////
    /// \class generic_component_factory generic_component_factory.hpp hpx/runtime/components/generic_component_factory.hpp
    ///
    /// The \a generic_component_factory provides a minimal implementation of a
    /// component's factory. If no additional functionality is required this
    /// type can be used to implement the full set of minimally required
    /// functions to be exposed by a component's factory instance. It supports
    /// creating objects requiring one constructor parameter.
    ///
    /// \tparam Component   The component type this factory should be
    ///                     responsible for.
    template <typename Component>
    struct generic_component_factory : public component_factory_base
    {
        /// \brief Construct a new factory instance
        ///
        /// \param global   [in] The pointer to a \a hpx#util#section instance
        ///                 referencing the settings read from the [settings]
        ///                 section of the global configuration file (hpx.ini)
        ///                 This pointer may be NULL if no such section has
        ///                 been found.
        /// \param local    [in] The pointer to a \a hpx#util#section instance
        ///                 referencing the settings read from the section
        ///                 describing this component type:
        ///                 [hpx.components.\<name\>], where \<name\> is the
        ///                 instance name of the component as given in the
        ///                 configuration files.
        ///
        /// \note The contents of both sections has to be cloned in order to
        ///       save the configuration setting for later use.
        generic_component_factory(util::section const* global,
                util::section const* local, bool isenabled)
          : isenabled_(isenabled), refcnt_(0)
        {
            // store the configuration settings
            if (NULL != global)
                global_settings_ = global->clone();
            if (NULL != local)
                local_settings_ = local->clone();
        }

        ///
        ~generic_component_factory() {}

        /// \brief Return the unique identifier of the component type this
        ///        factory is responsible for
        ///
        /// \param prefix       [in] The prefix of the locality this factory
        ///                     is responsible for.
        /// \param agas_client  [in] The AGAS client to use for component id
        ///                     registration (if needed).
        ///
        /// \return Returns the unique identifier of the component type this
        ///         factory instance is responsible for. This function throws
        ///         on any error.
        component_type get_component_type(naming::gid_type const& prefix,
            naming::resolver_client& agas_client)
        {
            typedef typename Component::type_holder type_holder;
            if (component_invalid == components::get_component_type<type_holder>())
            {
                // first call to get_component_type, ask AGAS for a unique id
                if (isenabled_) {
                    components::set_component_type<type_holder>(
                        (component_type) agas_client.register_factory(prefix,
                            unique_component_name<generic_component_factory>::call()));
                }
                else {
                    components::set_component_type<type_holder>(
                        (component_type) agas_client.get_component_id(
                            unique_component_name<generic_component_factory>::call()));
                }
            }
            return components::get_component_type<type_holder>();
        }

        /// \brief Return the name of the component type this factory is
        ///        responsible for
        ///
        /// \return Returns the name of the component type this factory
        ///         instance is responsible for. This function throws on any
        ///         error.
        std::string get_component_name() const
        {
            return unique_component_name<generic_component_factory>::call();
        }

        /// \brief  The function \a get_factory_properties is used to
        ///         determine, whether instances of the derived component can
        ///         be created in blocks (i.e. more than one instance at once).
        ///         This function is used by the \a distributing_factory to
        ///         determine a correct allocation strategy
        factory_property get_factory_properties() const
        {
            return Component::get_factory_properties();
        }

        /// \brief Create one or more new component instances.
        ///
        /// \param count  [in] The number of component instances to
        ///               create. The value of this parameter should not
        ///               be zero.
        ///
        /// \return Returns the GID of the first newly created component
        ///         instance. If more than one component instance has been
        ///         created (\a count > 1) the GID's of all new instances are
        ///         sequential in a row.
        naming::gid_type create (std::size_t count)
        {
            if (isenabled_) {
                naming::gid_type id = server::create<Component>(count);
                if (id)
                    ++refcnt_;
                return id;
            }

            HPX_THROW_EXCEPTION(bad_request,
                "generic_component_factory::create",
                "this factory instance is disabled for this locality (" +
                get_component_name() + ")");
            return naming::invalid_gid;
        }

        /// \brief Create one new component instance using the given constructor
        ///        argument.
        ///
        /// \param Arg0  [in] The type specific constructor argument
        ///
        /// \return Returns the GID of the first newly created component
        ///         instance. If more than one component instance has been
        ///         created (\a count > 1) the GID's of all new instances are
        ///         sequential in a row.
        naming::gid_type create_one (components::constructor_argument const&)
        {
            HPX_THROW_EXCEPTION(bad_request,
                "component_factory::create_one",
                "create_one is not supported by this factory instance");
            return naming::invalid_gid;
        }

        /// \brief Create one new component instance and initialize it using
        ///        the using the given constructor function.
        ///
        /// \param ctor  [in] The constructor function to call in order to
        ///         initialize the newly allocated object.
        ///
        /// \return Returns the GID of the first newly created component
        ///         instance. If more than one component instance has been
        ///         created (\a count > 1) the GID's of all new instances are
        ///         sequential in a row.
        naming::gid_type create_one_functor(HPX_STD_FUNCTION<void(void**)> const& ctor)
        {
            if (isenabled_) {
                naming::gid_type id = server::create_one<Component>(ctor);
                if (id)
                    ++refcnt_;
                return id;
            }

            HPX_THROW_EXCEPTION(bad_request,
                "generic_component_factory::create_one",
                "this factory instance is disabled for this locality (" +
                get_component_name() + ")");
            return naming::invalid_gid;
        }
    /**/

        /// \brief Destroy one or more component instances
        ///
        /// \param gid    [in] The gid of the first component instance to
        ///               destroy.
        void destroy(naming::gid_type const& gid)
        {
            server::destroy<Component>(gid);
            --refcnt_;
        }

        /// \brief Ask whether this factory can be unloaded
        ///
        /// \return Returns whether it is safe to unload this factory and
        ///         the shared library implementing this factory. This
        ///         function will return 'true' whenever no more outstanding
        ///         instances of the managed object type are alive.
        bool may_unload() const
        {
            return refcnt_ == 0;
        }

    protected:
        util::section global_settings_;
        util::section local_settings_;
        bool isenabled_;

        // count outstanding instances to avoid premature unloading
        boost::detail::atomic_count refcnt_;
    };
}}

///////////////////////////////////////////////////////////////////////////////
/// The macro \a HPX_REGISTER_MINIMAL_COMPONENT_FACTORY is used create and to
/// register a minimal component factory with Boost.Plugin. This macro may be
/// used if the registered component factory is the only factory to be exposed
/// from a particular module. If more than one factories need to be exposed
/// the \a HPX_REGISTER_COMPONENT_FACTORY and \a HPX_REGISTER_COMPONENT_MODULE
/// macros should be used instead.
#define HPX_REGISTER_MINIMAL_GENERIC_COMPONENT_FACTORY_EX(                    \
            ComponentType, componentname, enable_always)                      \
        HPX_REGISTER_COMPONENT_FACTORY(                                       \
            hpx::components::generic_component_factory<ComponentType>,        \
            componentname);                                                   \
        HPX_DEF_UNIQUE_COMPONENT_NAME(                                        \
            hpx::components::generic_component_factory<ComponentType>,        \
            componentname)                                                    \
        template struct hpx::components::generic_component_factory<ComponentType>;\
        HPX_REGISTER_MINIMAL_COMPONENT_REGISTRY_EX(                           \
            ComponentType, componentname, enable_always)                      \
    /**/

#define HPX_REGISTER_MINIMAL_GENERIC_COMPONENT_FACTORY(ComponentType, componentname) \
        HPX_REGISTER_MINIMAL_GENERIC_COMPONENT_FACTORY_EX(                    \
            ComponentType, componentname, false)                              \
    /**/

#endif