#pragma once

#include <string>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <mutex>

#include "marlin/Exceptions.h"

MARLIN_DEFINE_EXCEPTION( BookException );

using Flag_t = unsigned char;

//! \exception ObjectNotFinalized the object is not in a valid state to perform the action.
//! \exception ExternelError the action results in an Exception from an extern lib.

template<class HistT>
class HistHnd {
public:
  struct NullHnd {
    template<typename ... Ts>
    NullHnd(Ts ...) {}
  };
  using Type = NullHnd;
  static constexpr bool valid = false;
};


/*! magaed and store booked objects */
class BookStore {
public:
  enum struct Flags : Flag_t;
  enum struct State {Init, Processing, End} _state;
  using Count_t = unsigned char;
  using size_t = unsigned int;
private:
  struct Entrie {
    Count_t nrHistInstances;
    size_t begin;
    Flags flags;
    std::size_t typeHash;
    bool finalized;
  };
public:
  using EntrieMap = std::unordered_map
    <std::string,Entrie>;
private:
  const Count_t _maxInstances;

  EntrieMap  _pathToHist;

  std::vector<std::shared_ptr<void>> _hists;

  
  EntrieMap::iterator
  AddEntrie(
    const std::string& path,
    const Flags& flags,
    std::size_t typeHash);

public:
  void Finalize(Entrie& entrie) {
    entrie.finalized = true;
  };
  BookStore(Count_t maxInstances)
    : _maxInstances{maxInstances},
      _state{State::Init}
  {}
  void SetState(const State& state) { _state = state; }

  
  /** Modification Flags for Booking
   * \note not every Flag have for every Object a meaning
   */
  enum struct Flags : Flag_t {
    MultiInstances = 1 << 0, ///< use more memory to avoid mutex
    Default = 0b1
  };

  template<class BookT>
  typename Hnd<BookT>::Type
  Book(
    const std::string& path,
    const Flags& flags = Flags::Default
  );

  /** Book Object and return a Handle to it, 
     * if Object not alreadey exist, create a new one
   * \param path booking path for the Object
     * \param name of the instance
     * \param flags Flag to control the behavior
     */

};


template<class BookT>
HisHnd<BookT>
BookStore::HisHnd<BookT>(
  const std::filesystem::path&,
  const std::string&,
  const Flags&
) {
  static_assert(true, "Can't book object of this Type!");
}
inline BookStore::Flags
operator&(const BookStore::Flags& l, const BookStore::Flags& r) {
  return static_cast<BookStore::Flags>(
    static_cast<Flag_t>(l) & static_cast<Flag_t>(r));

}

template<class BookT>
typename Hnd<BookT>::Type
  BookStore::Book(
  const std::string& path,
  const Flags& flags
) {
  static_assert(Hnd<BookT>::valid, "Type is not Supported");

  if(_state != State::Init)
    MARLIN_THROW_T(BookException, "booking is only in init possible.");

  std::shared_ptr<BookT> ptr;

  auto itr =  _pathToHist.find(path);
  if (itr == _pathToHist.end()) {
    itr = AddEntrie(path, flags, typeid(BookT).hash_code());
  }

  if(itr->second.typeHash != typeid(BookT).hash_code()) {
    MARLIN_THROW_T(BookException, "Allrdeady booked with other Type.");
  }
  if(itr->second.flags != flags) {
    MARLIN_THROW_T(BookException, "Allready booked with other Flags.");
  }

  Count_t nr = itr->second.nrHistInstances ++;

  if(static_cast<bool>(
    itr->second.flags & Flags::MultiInstances)) {
    if(nr >= _maxInstances) {
      MARLIN_THROW_T(BookException, "created too many instances");
    }

    ptr = std::make_shared<BookT>();
    _hists[itr->second.begin + nr] = ptr;

  } else {

    if(nr == 1) {
      ptr = std::make_shared<BookT>();
      _hists[itr->second.begin] = ptr;
    } else {
      ptr = std::static_pointer_cast<BookT>
        (_hists[itr->second.begin]);
      itr->second.nrHistInstances -= 1;
    }
  }

  return typename Hnd<BookT>::Type(ptr, flags, itr->second, *this);

}

/*! \fn Hnd<BookT> BookStore::Book(const std::experimental::filesystem::path& path, const std::string& name, const Flags& fglas = Flags::Default)
 * \note <b>supported Types</b>
 *  - RH1D
 *  - RH1F
 *  - RH1C
 *  - RH1I
 *  - RH1LL
 */
