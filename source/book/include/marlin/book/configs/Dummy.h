#pragma once

namespace marlin {
	namespace book {
		namespace types {
			
			template<typename Config>
			HistT<Config>& add(HistT<Config>& to, const HistT<Config>& from) {
				return to;
			}

			template<typename Config>
			void add(
				const std::shared_ptr<HistT<Config>>& to,
				const std::shared_ptr<HistT<Config>>& from){}

			template<typename Config>
			HistT<Config>::HistT(
					const std::string_view& title,
					const AxisConfig<typename Config::Precision_t>& axis) {}

			template<typename Config>
			HistT<Config>::HistT(
					const std::string_view& title,
					const AxisConfig<typename Config::Precision_t>& axisA,
					const AxisConfig<typename Config::Precision_t>& axisB) {}

			template<typename Config>
			HistT<Config>::HistT(
					const std::string_view& title,
					const AxisConfig<typename Config::Precision_t>& axisA,
					const AxisConfig<typename Config::Precision_t>& axisB,
					const AxisConfig<typename Config::Precision_t>& axisC) {}

			template<typename Config>
			void HistT<Config>::Fill(const Point_t& p, const Weight_t& w){}

			template< typename Config >
			void HistT<Config>::FillN(const Point_t *pFirst, const Point_t *pLast,
								const Weight_t *wFirst, const Weight_t *wLast){}

			template< typename Config >
			void HistT<Config>::FillN(const Point_t *first, const Point_t *last){}

			template< typename Config >
			HistConcurrentFillManager<Config>::HistConcurrentFillManager(HistT<Config>& hist) {}

			template< typename Config >
			HistConcurrentFiller<Config>::HistConcurrentFiller(HistConcurrentFillManager<Config>& mgr){}

			template< typename Config >
			void HistConcurrentFiller<Config>::Fill(const Point_t& p, const Weight_t& w){}

			template< typename Config >
			void HistConcurrentFiller<Config>::FillN(const Point_t *pFirst, const Point_t *pLast,
								const Weight_t *wFirst, const Weight_t *wLast){}

			template< typename Config >
			void HistConcurrentFiller<Config>::FillN(const Point_t *first, const Point_t *last){}

			template<typename Config>
			void HistConcurrentFiller<Config>::Flush() {}

			template<typename Config>
			auto toRoot6(const HistT<Config>& hist, const std::string_view& name) {
				return nullptr;
			}


			using H1F = HistT<HistConfig<double, float , 1>>;
			using H1D = HistT<HistConfig<double, double , 1>>;
			using H1I = HistT<HistConfig<double, int , 1>>;
			using H2F = HistT<HistConfig<double, float , 2>>;
			using H2D = HistT<HistConfig<double, double , 2>>;
			using H2I = HistT<HistConfig<double, int , 2>>;
			using H3F = HistT<HistConfig<double, float , 3>>;
			using H3D = HistT<HistConfig<double, double , 3>>;
			using H3I = HistT<HistConfig<double, int , 3>>;

		} // end namespace types
	} // end namespace book
} // end namespace marlin
