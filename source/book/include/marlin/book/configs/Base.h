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
			/**
			 *	@brief collection for Axis Description 
			 */
      template<typename Precision_t>
      class AxisConfig {
      public:

				/**
				 *	@brief Axis with equal sized bins. 
				 *	@param title of the axis
				 *	@param bins amount of bins
				 *	@param min lower limit 
				 *	@param max upper limit
				 */
				AxisConfig( const std::string_view& title,
                    std::size_t bins,
                    Precision_t min,
                    Precision_t max) 
          : _title(title), _bins{bins}, _min{min}, _max{max}{}


				/**
				 *	@brief Axis with equal sized bins. 
				 *	@param bins number of bins
				 *	@param min lower limit
				 *	@param max upper limit
				 */
				AxisConfig( std::size_t bins,
				            Precision_t min,
				            Precision_t max) 
          : AxisConfig( "", bins, min, max ){}

				/**
				 *	@brief Axis with irregular borders 
				 *	@param title title
				 *	@param borders list of borders,
				 *	       - borders[0] … lower bound
				 *	       - borders.end … upper bound
				 *	       - borders.size - 1 … amount of bins
				 */
				AxisConfig( const std::string_view& title,
                    std::vector<Precision_t>&& borders) 

          : _title(title),
            _bins{borders.size() - 1},
            _min{borders[0]},
            _max{borders.end()},
            _iregularBorder{borders}
        {}

				/**
				 *	@brief Axis with irregular borders 
				 *	@param borders list of borders,
				 *	       - borders[0] … lower bound
				 *	       - borders.end … upper bound
				 */
        AxisConfig( std::vector<Precision_t>&& borders) 
          : AxisConfig( "", borders ){}

				/// Get Axis Title.
        const std::string_view title() const { return _title; }

				/// Get amount of bins.
        std::size_t bins() const { return _bins; }

				/// Get lower bound.
        Precision_t min() const { return _min; }

				/// Get upper bound.
        Precision_t max() const { return _max; }

				/// check if bins are equal sized. 
        bool isRegular() const { return !_iregularBorder.has_value(); }

				/** 
				 * 	@brief get borders, when borders are irregular.
				 *	@return vector of irregular borers, or empty vector if bins equal sized. 
				 */
        const std::vector<Precision_t> iregularBorder() const {
          return _iregularBorder.value_or(std::vector<Precision_t>{});
        }
      private:
        std::string _title;
        std::size_t _bins;
        Precision_t _min;
        Precision_t _max;
        std::optional<std::vector<Precision_t>> _iregularBorder{std::nullopt};
      };


			/**
			 *	@brief type trait for Histograms.
			 *	Define types needed for general described Histogram.
			 */
      template<typename P, typename W, std::size_t D>
      struct HistConfig {
				/// type used for bin weight
        using Weight_t = W;
				/// type used for bin borders
        using Precision_t = P;
				/// math type used to handle histogram
        using Impl_t = void*;
				/// class to access single instance across multiple threads. 
        using ConcurrentFiller_t = void*;
				/// class to create ConcurrentFiller for one Histogram. 
        using ConcurrentManager_t = void*;
        static constexpr std::size_t Dimension = static_cast<std::size_t>(D);
      };

      template<typename>
      class HistT;

			/// add weights from one Histogram to an other.
      template<typename Config>
      HistT<Config>& add(HistT<Config>& to, const HistT<Config>& from);

			/// \see HistT<Config>& add(HistT<Config>& to, const HistT<Config>& from);
      template<typename Config>
      void add(
          const std::shared_ptr<HistT<Config>>& to,
          const std::shared_ptr<HistT<Config>>& from);

      template<typename>
      class HistConcurrentFiller;

      template<typename>
      class HistConcurrentFillManager;

			/**
			 *	@brief Generalized histogram class. 
			 */
      template<typename Config>
      class HistT {
        typename Config::Impl_t& impl() { return _impl; }
        const typename Config::Impl_t& impl() const { return _impl; }
        friend HistT<Config>& add<Config>(HistT<Config>&,const HistT<Config>&);
        friend void add<Config>(const std::shared_ptr<HistT<Config>>&,const std::shared_ptr<HistT<Config>>&);
        friend class HistConcurrentFillManager<Config>;

      public:
				/// type used for bin weight
        using Weight_t = typename Config::Weight_t;
				/// type used for bin borders
        using Precision_t = typename Config::Precision_t;
				/// Dimension of the histogram
        static constexpr std::size_t Dimension = Config::Dimension;
				/// type used for Entry Points
        using Point_t = std::array<Precision_t, Dimension>;
				/// types used to configure Axis 
        using AxisConfig_t = AxisConfig<Precision_t>;

				/**
				 *	@brief non-title 1D-histogram constructor. Only available for 1D-histograms. 
				 */
        HistT(const AxisConfig_t& axis) : HistT("",axis) {}

				/**
				 *	@brief non-title 2D-histogram constructor. Only available for 2D-histograms. 
				 */
        HistT(const AxisConfig_t& axisA,
							const AxisConfig_t& axisB) : HistT("",axisA, axisB) {}

				/**
				 *	@brief non-title 3D-histogram constructor. Only available for 3D-histograms. 
				 */
        HistT(const AxisConfig_t& axisA,
							const AxisConfig_t& axisB,
							const AxisConfig_t& axisC) : HistT("",axisA, axisB, axisC) {}

				/**
				 *	@brief Titled 1D-histogram constructor. Only available for 1D-histograms.
				 */
        HistT( const std::string_view& title, const AxisConfig_t& axis);

				/**
				 *	@brief Titled 2D-histogram constructor. Only available for 2D-histograms.
				 */
        HistT(
            const std::string_view& title,
            const AxisConfig_t& axisA,
            const AxisConfig_t& axisB);

				/**
				 *	@brief Titled 3D-histogram constructor. Only available for 3D-histograms.
				 */
        HistT( const std::string_view& title,
               const AxisConfig_t& axisA,
               const AxisConfig_t& axisB,
               const AxisConfig_t& axisC);

				/**
				 *	@brief Add one weighted point to histogram. 
				 */
				void Fill(const Point_t& point, const Weight_t& weight);

				// TODO: used generic iterator
				/**
				 *	@brief add multiple weighted points to histogram. 
				 *	@note data must be available as stripe 
				 *	@param pFirst address of first point
				 *	@param pLast address of last point (not included)
				 *	@param wFirst address of first weight
				 *	@param wLast address of last weight (not included)
				 */
				void FillN(const Point_t *pFirst, const Point_t *pLast,
                  const Weight_t *wFirst, const Weight_t *wLast);


				/**
				 *	@brief add multiple one-weighted points to histogram. 
				 *	@note data must be available as stripe 
				 *	@param pFirst address of first point
				 *	@param pLast address of last point (not included)
				 */
        void FillN(const Point_t *first, const Point_t *last);

				/**
				 *	@brief get read access to actual implementation. 
				 */
        const typename Config::Impl_t& get() const { return _impl; }

				/**
				 *	@brief check if histogram type implemented. 
				 */
        constexpr bool hasImpl() {
          return !std::is_same_v<Config::Impl_t, void*>;
        }

      protected:
          typename Config::Impl_t _impl{};
      };

			/**
			 *	@brief class managing HistConcurrentFiller creation for one histogram.
			 */
      template<typename Config>
      class HistConcurrentFillManager {
        friend class HistConcurrentFiller<Config>;
        typename Config::ConcurrentManager_t& impl() { return _impl; }
      public:
        explicit  HistConcurrentFillManager(HistT<Config>& hist);
      private:
        typename Config::ConcurrentManager_t _impl;
      };

			/**
			 *	@brief class managing parallel filling to one histogram.
			 */
      template<typename Config>
      class HistConcurrentFiller {
      public:
				/// managed Histogram type.
        using Type = HistT<Config>;
				/// Type used for points.
        using Point_t = typename Type::Point_t;
				/// Type used for bin weight.
        using Weight_t = typename Type::Weight_t;

        explicit HistConcurrentFiller(HistConcurrentFillManager<Config>& manager);

				/// \see void Hist<Config>::Fill(const Point_t& p, const Weigh_t& w)
        void Fill(const Point_t& p, const Weight_t& w);

        /// \see void FillN(const Point_t *pFirst, const Point_t *pLast, const Weight_t *wFirst, const Weight_t *wLast);
        void FillN(const Point_t *pFirst, const Point_t *pLast,
                  const Weight_t *wFirst, const Weight_t *wLast);

        /// \see void FillN(const Point_t *first, const Point_t *last);
        void FillN(const Point_t *first, const Point_t *last);

				/// flushed may buffer data.
        void Flush();
      
      private:
        typename Config::ConcurrentFiller_t _impl;
      };

			/**
			 *	@brief convert histogram to Root-6 Object for serialization 
			 *	@param hist histogram to convert
			 *	@param name of object.
			 */
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
