#pragma once

// -- std includes
#include <type_traits>
#include <optional>
#include <vector>
#include <array>
#include <string_view>
#include <string>
#include <initializer_list>
#include <memory>

namespace marlin {
  namespace book {
    /// Alias for Types used by MarlinBook
    namespace types {
  
      // TODO: check for no copy !!
      template<typename Precision_t>
      class AxisConfig {
      public:

        AxisConfig( const std::string_view& title,
                    std::size_t bins,
                    Precision_t min,
                    Precision_t max) 
          : _title(title), _bins{bins}, _min{min}, _max{max}{}

        AxisConfig( std::size_t bins,
                    Precision_t min,
                    Precision_t max) 
          : AxisConfig( "", bins, min, max ){}

        AxisConfig( const std::string_view& title,
                    std::vector<Precision_t>&& borders) 
          : _title(title),
            _bins{borders.size() - 1},
            _min{borders[0]},
            _max{borders.end()},
            _iregularBorder{borders}
        {}

        AxisConfig( std::vector<Precision_t>&& borders) 
          : AxisConfig( "", borders ){}

        const std::string_view title() const { return _title; }
        std::size_t bins() const { return _bins; }
        Precision_t min() const { return _min; }
        Precision_t max() const { return _max; }
        bool isIregular() const { return _iregularBorder.has_value(); }
        const std::vector<Precision_t> iregularBorder() const {
          return _iregularBorder.value();
        }
      private:
        std::string _title;
        std::size_t _bins;
        Precision_t _min;
        Precision_t _max;
        std::optional<std::vector<Precision_t>> _iregularBorder{std::nullopt};
      };


      template<typename P, typename W, std::size_t D>
      struct HistConfig {
        using Weight_t = W;
        using Precision_t = P;
        using Impl_t = void;
        using ConcurrentModifiyer_t = void;
        using ConcurrentManager_t = void;
        static constexpr std::size_t Dimension = static_cast<std::size_t>(D);
      };

      template<typename>
      class HistT;

      template<typename Config>
      HistT<Config>& add(HistT<Config>& to, const HistT<Config>& from);

      template<typename Config>
      void add(
          const std::shared_ptr<HistT<Config>>& to,
          const std::shared_ptr<HistT<Config>>& from);

      template<typename>
      class HistConcurrentFiller;
      template<typename>
      class HistConcurrentFillManager;

      template<typename Config>
      class HistT {
        typename Config::Impl_t& impl() { return _impl; }
        const typename Config::Impl_t& impl() const { return _impl; }
        friend HistT<Config>& add<Config>(HistT<Config>&,const HistT<Config>&);
        friend void add<Config>(const std::shared_ptr<HistT<Config>>&,const std::shared_ptr<HistT<Config>>&);
        friend class HistConcurrentFillManager<Config>;
      public:
        using Weight_t = typename Config::Weight_t;
        using Precision_t = typename Config::Precision_t;
        static constexpr std::size_t Dimension = Config::Dimension;
        using Point_t = std::array<Precision_t, Dimension>;
        using AxisConfig_t = AxisConfig<Precision_t>;

        HistT(const AxisConfig_t& axis) : HistT("",axis) {}

        HistT( const std::string_view& title, const AxisConfig_t& axis);
        HistT(
            const std::string_view& title,
            const AxisConfig_t& axisA,
            const AxisConfig_t& axisB);
        HistT( const std::string_view& title,
               const AxisConfig_t& axisA,
               const AxisConfig_t& axisB,
               const AxisConfig_t& axisC);

        void Fill(const Point_t& p, const Weight_t& w);

        void FillN(const Point_t *pFirst, const Point_t *pLast,
                  const Weight_t *wFirst, const Weight_t *wLast);

        void FillN(const Point_t *first, const Point_t *last);

        const typename Config::Impl_t& get() const { return _impl; }

        constexpr bool hasImpl() {
          return !std::is_same_v<Config::Impl_t, void>;
        }

      protected:
          typename Config::Impl_t _impl{};
      };

      template<typename Config>
      class HistConcurrentFillManager {
        friend class HistConcurrentFiller<Config>;
        typename Config::ConcurrentManager_t& impl() { return _impl; }
      public:
        explicit  HistConcurrentFillManager(HistT<Config>& hist);
      private:
        typename Config::ConcurrentManager_t _impl;
      };

      template<typename Config>
      class HistConcurrentFiller {
      public:
        using Type = HistT<Config>;
        using Point_t = typename Type::Point_t;
        using Weight_t = typename Type::Weight_t;

        explicit HistConcurrentFiller(HistConcurrentFillManager<Config>& manager);

        void Fill(const Point_t& p, const Weight_t& w);

        void FillN(const Point_t *pFirst, const Point_t *pLast,
                  const Weight_t *wFirst, const Weight_t *wLast);

        void FillN(const Point_t *first, const Point_t *last);

        void Flush();
      
      private:
        typename Config::ConcurrentModifiyer_t _impl;
      };

      template<typename Config>
      auto toRoot6(const HistT<Config>& hist, const std::string_view& name);


    } // end namespace types
    
    // -- MarlinBook forward declarations
    class BookStore ;
    template < typename T >
    class Handle ;
    template < typename T >
    class BaseHandle ;

  } // end namespace book
} // end namespace marlin
