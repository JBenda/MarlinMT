	template<typename Precision_t>
			class AxisConfig {
			public:
				AxisConfig("", bins, min,max);
				AxisConfig("", std::vector<Precsision>&& binborders)
				// getters
			private:
				std::string title;
				std::vector<Prcision_t> binborders;
				bool isIregular;
			}

			template<typename Config> 
			class HistT{
			public:
				using Impl_t Config::Impl_t; // FIXME: not for user
				using Weight_t =  = Config::Weight_t;
				using Precision_t = Config::Precsion_t;
				static constexpr Dimension = Config::Dimension;
				using Point_t = std::array<Precision_t, Dimension>;
				using AxicConfig_t = AxicsConfig<Precision_t>;
			

				HistT("title", AxisConfig_t) { static_assert(Dimension == 1); …} // 1D
				HistT("title", AxisConfig_t, AxisConfig_t) { … }; // 2D
				HistT("title", AxisConfig_t, AxisConfig_t, AxisConfig<…>) {…}; // 3D

				void Fill(const Point_t& p, const Weight_t& w);

				template<typename Point_itr, typename Weigh_Itr>
				void FillN(Point_itr first, Pint_itr last, Weight_Itr begin, Weight_Itr end);
				// check if it really pointer

				template<typename Point_itr >
				void FillN(Point_itr first, Pint_itr last);

				// getters
				const Impl_t& get() const { return impl; }

				static constexpr bool hasImpl() {
					return std::is_same_v<Impl_t, void>;
				}
			};



