#pragma once

// -- std header
#include <memory>
#include <utility>
#include <vector>

namespace marlin {
  namespace book {

    /**
     *  @brief MemLayout base class to store booked objects in MarlinMT
     */
    class MemLayout {
    public:
      /**
       *  @brief Get Resource for an instance.
       *  @param idx instance id
       *  @return const pointer to Resource
       */
      template < typename T >
      [[nodiscard]] std::shared_ptr< const T > at( std::size_t idx ) const {
        return std::static_pointer_cast< const T >( impAt( idx ) ) ;
      }

      /**
       *  @brief Get Resource for an instance.
       *  @param idx instance id
       *  @return pointer to Resource
       */
      template < typename T >
      std::shared_ptr< T > at( std::size_t idx ) {
        return std::static_pointer_cast< T >( impAt( idx ) ) ;
      }

      /**
       *  @brief Get Completed Resource. Same for every Instance.
       *  @note may recalculation for every call
       */
      template < typename T >
      std::shared_ptr< const T > merged() {
        return std::static_pointer_cast< const T >( impMerged() ) ;
      }

      MemLayout()                               = default ;
      MemLayout( const MemLayout & )            = default ;
      MemLayout &operator=( const MemLayout & ) = default ;
      MemLayout( MemLayout && )                 = default ;
      MemLayout &operator=( MemLayout && )      = default ;
      virtual ~MemLayout()                      = default ;

    protected:
      /// implementation from at
      [[nodiscard]] virtual std::shared_ptr< void >
      impAt( std::size_t idx ) const = 0 ;
      /// implementation from merged
      [[nodiscard]] virtual std::shared_ptr< void > impMerged() = 0 ;
    } ;

    /**
     *  @brief MemLayout for mutable object instances.
     *  @tparam T stored Object Type
     *  @tparam MERGE function(to, from) which merge to instances of Object
     */
    template < typename T,
               void ( *MERGE )( const std::shared_ptr< T > & /* dst */,
                                const std::shared_ptr< T > & /* src */
                                ),
               typename... Args_t >
    class SharedMemLayout : public MemLayout {

    public:
      /**
       *  @brief Constructor.
       *  @param amount of instances amount of Resource Instances
       *  @param args Arguments for Object Construction
       */
      explicit SharedMemLayout( std::size_t num_instances, Args_t... args )
        : _objects{num_instances, nullptr},
          _ctor_p{
            std::make_unique< typename decltype( _ctor_p )::element_type >(
              args... )} {
        for ( std::shared_ptr< T > &pObj : _objects ) {
          pObj = std::make_shared< T >( args... ) ;
        }
      }

      SharedMemLayout( const SharedMemLayout & )                = default ;
      SharedMemLayout &operator=( const SharedMemLayout & )     = default ;
      SharedMemLayout( SharedMemLayout && ) noexcept            = default ;
      SharedMemLayout &operator=( SharedMemLayout && ) noexcept = default ;
      ~SharedMemLayout() override final                         = default ;

    private:
      /// Get Resource for Instance. Lazy operation.
      [[nodiscard]] std::shared_ptr< void >
      impAt( std::size_t idx ) const override final {
        return _objects[idx] ;
      }

      [[nodiscard]] std::shared_ptr< void > impMerged() override final {
        _mergedObj
          = std::make_shared< T >( std::make_from_tuple< T >( *_ctor_p ) ) ;
        for ( std::shared_ptr< T > &pObj : _objects ) {
          if ( pObj ) {
            ( *MERGE )( _mergedObj, pObj ) ;
          }
        }
        return _mergedObj ;
      }

      std::vector< std::shared_ptr< T > >
                           _objects; 
      std::shared_ptr< T > _mergedObj{nullptr} ;
      std::unique_ptr<
        std::tuple< const typename std::remove_reference< Args_t >::type... > >
        _ctor_p ;
    } ;

    /**
     *  @brief MemLayout for Single object instance.
     *  @tparam T stored Object Type
     */
    template < typename T, typename... Args_t >
    class SingleMemLayout : public MemLayout {
    public:
      explicit SingleMemLayout( Args_t... args )
        : _object{std::make_shared< T >( args... )} {}

      SingleMemLayout( const SingleMemLayout & )                = default ;
      SingleMemLayout &operator=( const SingleMemLayout & )     = default ;
      SingleMemLayout( SingleMemLayout && ) noexcept            = default ;
      SingleMemLayout &operator=( SingleMemLayout && ) noexcept = default ;
      ~SingleMemLayout() override final                         = default ;

    private:
      [[nodiscard]] std::shared_ptr< void >
      impAt( std::size_t /*idx*/ ) const override final {
        return _object ;
      }

      /// @note cheap merge
      [[nodiscard]] std::shared_ptr< void > impMerged() override final {
        return _object ;
      }

      std::shared_ptr< T > _object{nullptr} ;
    } ;

  } // end namespace book
} // end namespace marlin
