#pragma once // TOOD: not used to detect double binding

#include "marlin/book/configs/Base.h"

// -- ROOT includes
#include "RVersion.h"

#include "ROOT/RHist.hxx"
#include "ROOT/RHistConcurrentFill.hxx"
#include "ROOT/RHistData.hxx"
#include "ROOT/RSpan.hxx"
#include "ROOT/RFile.hxx"
#include "ROOT/RDirectory.hxx"

// -- histogram conversion include 
#include "marlin/book/RootHistV7ToV6Conversion.h"

namespace marlin {
  namespace book {
    namespace types {

      template<typename Config>
      auto toRoot6(const HistT<Config>& hist, const std::string_view& name){
        return into_root6_hist(hist.get(), std::string(name).c_str());
      }

      template<typename T = double>
      class AxisConfig;

#ifndef MARLIN_HIST_FILLER_BUFFER_SIZE
# define MARLIN_HIST_FILLER_BUFFER_SIZE 1024
#endif
      /**
       *  @brief Buffer size for Histograms used in Shared mode.
       *  - larger →  less synchronisation points
       *  - larger →  more memory consumption.
       *  @note can set with the CMAKE  Variable \cod
       * {MARLIN_HIST_FILLER_BUFFER_SIZE}
       */
      constexpr std::size_t HistogramFillerBufferSize
        = MARLIN_HIST_FILLER_BUFFER_SIZE ;

#define HistConfig_ROOT(Alias, Impl, Weight, Dim) \
      template<>\
      struct HistConfig<double, Weight, Dim> {\
        using Weight_t = Weight;\
        using Precision_t = double;\
        using Impl_t = Impl;\
        using ConcurrentModifiyer_t \
          = ROOT::Experimental::RHistConcurrentFiller<Impl, HistogramFillerBufferSize>;\
        using ConcurrentManager_t \
          = ROOT::Experimental::RHistConcurrentFillManager<Impl, HistogramFillerBufferSize>;\
        static constexpr std::size_t Dimension = static_cast<std::size_t>(Dim);\
      };\
      using Alias = HistT<HistConfig<double, Weight, Dim>>

      HistConfig_ROOT(H1F, ROOT::Experimental::RH1F, float, 1);
      HistConfig_ROOT(H1D, ROOT::Experimental::RH1F, double, 1);
      HistConfig_ROOT(H1I, ROOT::Experimental::RH1F, int, 1);
      HistConfig_ROOT(H2F, ROOT::Experimental::RH2F, float, 2);
      HistConfig_ROOT(H2D, ROOT::Experimental::RH2F, double, 2);
      HistConfig_ROOT(H2I, ROOT::Experimental::RH2F, int, 2);
      HistConfig_ROOT(H3F, ROOT::Experimental::RH3F, float, 3);
      HistConfig_ROOT(H3D, ROOT::Experimental::RH3F, double, 3);
      HistConfig_ROOT(H3I, ROOT::Experimental::RH3F, int, 3);


      template<typename Config>
      HistT<Config>::HistT(
          const std::string_view& title,
          const AxisConfig<typename Config::Precision_t>& axis) 
        : _impl(
            title,
            ROOT::Experimental::RAxisConfig(
              axis.title(),
              axis.bins(),
              axis.min(),
              axis.max()))
      {
        static_assert(std::is_same_v<typename Config::Precision_t, double>);
        static_assert(Dimension == 1);
      }

      template<typename Config>
      HistT<Config>::HistT(
          const std::string_view& title,
          const AxisConfig<typename Config::Precision_t>& axisA,
          const AxisConfig<typename Config::Precision_t>& axisB) 
        : _impl(
            title,
            ROOT::Experimental::RAxisConfig(
              axisA.title(),
              axisA.bins(),
              axisA.min(),
              axisA.max()),
            ROOT::Experimental::RAxisConfig(
              axisB.title(),
              axisB.bins(),
              axisB.min(),
              axisB.max()
            ))
      {
        static_assert(std::is_same_v<typename Config::Precision_t, double>);
        static_assert(Dimension == 2);
      }

      template<typename Config>
      HistT<Config>::HistT(
          const std::string_view& title,
          const AxisConfig<typename Config::Precision_t>& axisA,
          const AxisConfig<typename Config::Precision_t>& axisB,
          const AxisConfig<typename Config::Precision_t>& axisC) 
        : _impl(
            title,
            ROOT::Experimental::RAxisConfig(
              axisA.title(),
              axisA.bins(),
              axisA.min(),
              axisA.max()),
            ROOT::Experimental::RAxisConfig(
              axisB.title(),
              axisB.bins(),
              axisB.min(),
              axisB.max()),
            ROOT::Experimental::RAxisConfig(
              axisC.title(),
              axisC.bins(),
              axisC.min(),
              axisC.max())
            )
      {
        static_assert(std::is_same_v<typename Config::Precision_t, double>);
        static_assert(Dimension == 3);
      }

      template<std::size_t D>
      ROOT::Experimental::Hist::RCoordArray<D> toConfig(const std::array<double, D>& arr) {
        if constexpr(D == 1) {
          return ROOT::Experimental::Hist::RCoordArray<1>(arr[0]);
        } else if constexpr (D == 2) {
          return ROOT::Experimental::Hist::RCoordArray<2>(arr[0], arr[1]);
        } else if constexpr (D == 3) {
          return ROOT::Experimental::Hist::RCoordArray<3>(arr[0], arr[1], arr[2]);
        }
      }


      template<typename Config>
      void HistT<Config>::Fill(
          const typename HistT<Config>::Point_t& p,
          const typename HistT<Config>::Weight_t& w) {
        _impl.Fill(toConfig(p), w);
        static_assert(std::is_same_v<typename Config::Precision_t, double>);
      }

      template<typename Config>
      void HistT<Config>::template FillN(
        const typename HistT<Config>::Point_t* pFirst,
        const typename HistT<Config>::Point_t* pLast,
        const typename HistT<Config>::Weight_t* wFirst,
        const typename HistT<Config>::Weight_t* wLast
        ) {
        // _impl.FillN({pFirst, pLast}, {wFirst, wLast});
        auto p = pFirst;
        auto w = wFirst;
        for(;p != pLast && w != wLast; ++p,++w) {
          this->Fill(*p, *w);
        }
        static_assert(std::is_same_v<typename Config::Precision_t, double>);
      }

      template<typename Config>
      void HistT<Config>::FillN(
        const typename HistT<Config>::Point_t* first,
        const typename HistT<Config>::Point_t* last ) {
        _impl.FillN({first, last});
        static_assert(std::is_same_v<typename Config::Precision_t, double>);
      }

      template<typename Config>
      HistT<Config>& add(HistT<Config>& to, const HistT<Config>& from) {
        ROOT::Experimental::Add(to.impl(), from.impl());
        return to;
      }

      template<typename Config>
      void add(
          const std::shared_ptr<HistT<Config>>& to,
          const std::shared_ptr<HistT<Config>>& from) {
        ROOT::Experimental::Add(to->impl(), from->impl());
      }

      template<typename Config>
      HistConcurrentFillManager<Config>::HistConcurrentFillManager(
          HistT<Config>& hist)
        : _impl(hist.impl()){}

      template<typename Config>
      HistConcurrentFiller<Config>::HistConcurrentFiller(
          HistConcurrentFillManager<Config>& manager
          ) : _impl(manager.impl()){}
      
      template<typename Config>
      void HistConcurrentFiller<Config>::Fill(
          const typename HistT<Config>::Point_t& p,
          const typename HistT<Config>::Weight_t& w) {
        _impl.Fill(toConfig(p), w);
        static_assert(std::is_same_v<typename Config::Precision_t, double>);
      }

      template<typename Config>
      void HistConcurrentFiller<Config>::template FillN(
        const typename HistT<Config>::Point_t* pFirst,
        const typename HistT<Config>::Point_t* pLast,
        const typename HistT<Config>::Weight_t* wFirst,
        const typename HistT<Config>::Weight_t* wLast
        ) {
        // _impl.FillN({pFirst, pLast}, {wFirst, wLast});
        auto p = pFirst;
        auto w = wFirst;
        for(;p != pLast && w != wLast; ++p,++w) {
          this->Fill(*p, *w);
        }
        static_assert(std::is_same_v<typename Config::Precision_t, double>);
      }

      template<typename Config>
      void HistConcurrentFiller<Config>::Flush() {
        _impl.Flush();
      }

    } // end namespace types
  } // end namespace book
} // end namespace marlin

