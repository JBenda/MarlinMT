#include "marlin/book/Selection.h"

// -- MarlinBook includes
#include "marlin/book/BookStore.h"

namespace marlin {
  namespace book {
    WeakEntry::WeakEntry( const std::shared_ptr< const details::Entry > &entry )
      : _entry{entry}{}

    WeakEntry::WeakEntry( const std::shared_ptr< details::Entry > &entry )
      : _entry{entry}{}

    bool WeakEntry::valid() const {
      return !_entry.expired() && _entry.lock()->valid(); 
    }

    const EntryKey& WeakEntry::key() const {
      return _entry.lock()->key() ;
    }

    const Condition& Selection::condition() const { return _condition; }


    Selection::const_iterator Selection::begin() const {
      return const_iterator( _entries.cbegin() ) ;
    }
    
    Selection::const_iterator Selection::end() const {
      return const_iterator( _entries.cend() ) ;
    }
    
    std::size_t Selection::size() const { return _entries.size(); }

    Selection::Selection( const Selection &sel,
                          const Condition &cond,
                          ComposeStrategy  strategy )
      : Selection{find( sel.begin(), sel.end(), cond )} {
      switch ( strategy ) {
        case ComposeStrategy::AND:
          _condition = sel.condition() & cond ;
          break ;
        case ComposeStrategy::ONLY_CHILD:
          _condition = cond ;
          break ;
        case ComposeStrategy::ONLY_PARENT:
          _condition = sel.condition() ;
          break ;
        default:
          MARLIN_BOOK_THROW( "Condition compose strategy is not defined." ) ;
      }
    }

    Selection Selection::find( const Condition &cond,
                               ComposeStrategy  strategy ) {
      return Selection( *this, cond, strategy ) ;
    }

    //--------------------------------------------------------------------------

    void Selection::remove( std::size_t id ) {
      _entries.erase( _entries.cbegin() + id ) ;
    }

    //--------------------------------------------------------------------------

    void Selection::remove( std::size_t id, std::size_t n ) {
      auto beg = _entries.cbegin() + id ;
      _entries.erase( beg, beg + n ) ;
    }

    //--------------------------------------------------------------------------

    void Selection::remove( const const_iterator &itr ) {
      _entries.erase( itr ) ;
    }

    //--------------------------------------------------------------------------

    void Selection::remove( const const_iterator &begin,
                            const const_iterator &end ) {
      _entries.erase( begin, end ) ;
    }

    //--------------------------------------------------------------------------

    template Selection Selection::find< Selection::const_iterator >(
      Selection::const_iterator begin,
      Selection::const_iterator end,
      const Condition &         cond ) ;

  } // end namespace book
} // end namespace marlin
