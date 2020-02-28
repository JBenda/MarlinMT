Class for management global instances of data sinks (e.g. histograms, trees).


See [process diagram](/diagrams/BookStore-Schema.html) for further information.
[![process diagram](/diagrams/BookStore-Schema.svg)](/diagrams/BookStore-Schema.html)

## minimal example  

```cpp
class MyProcessor : public Processor {
	// ...
private:
	H1FEntry _hist; // 1D histogram with float weights
};

void MyProcessor::init() {
	_hist = ProcessorApi::Book::bookH1F(
		this,
		"/path/in/result/file/",
		"name",
		"the histogram title",
		{"axis title", nBins, min, max} // AxisConfiguration
	);	
}

void MyProcessor::processEvent(EventStore* event)
	H1FHandle histHandle = _hist.handle();
	for(/*...*/) {
		histHandle.fill(
			{x}, // point type: e.g.  '{x,y}' for 2D, etc.
			weight);
	}
}
```

## store histograms in array 

```cpp
class MyProcessor : public Processor {
	// ...
private:
	std::array<Hist1F, 3> _hist1fs; // 1D histograms with float weights
};

void MyProcessor::init() {
	int i = 0;
	for(auto& hist : _hist1fs) {
		hist = ProcessorApi::Book::bookH1F(
			this,
			"/path/in/reslult/file/" + std::to_string(i++) + "/",
			"name",
			"the histogram title",
			{"axis title", nBins, min, max} // AxisConfiguration
		);	
	}
}

void MyProcessor::processEvent(EventStore* event)

	std::array<H1FHandle, _hist1fs.size()> histHandles;
	std::transform(_hist1fs.begin(), _hist1fs.end(),
		histHandle.begin(), [](auto& hist){return hist.hanlde();});

	for(/*...*/) {
		histHandles[i].fill(
			{x}, // point type: e.g.  '{x,y}' for 2D, etc.
			weight);
	}
}
```

## store histograms in vector

```cpp
class MyProcessor : public Processor {
	// ...
private:
	std::vector<Hist2F> _hist2fs; // 2D histograms with float weights
};

void MyProcessor::init() {
	for(int i = 0; i < 10; ++i) {
		_hist2fs.emplace_back(
			ProcessorApi::Book::bookH2F(
				this,
				"/path/" + std::to_string(i) + "/",
				"name",
				"title",
				{"x", nBins, min, max},
				{"y", nBins, min, max}
			)
		);
	}
}

void MyProcessor::processEvent(EventStore* event)

	std::vector<H2FHandle> histHandles;
	std::transform(_hist2fs.begin(), _hist2fs.end(),
		std::bak_inserter(histHandle), [](auto& hist){return hist.hanlde();});

	for(/*...*/) {
		histHandles[i].fill(
			{x,y},
			weight);
	}
}

```

## book functions

See [reference manual](/doxygen/classmarlin_1_1ProcessorApi_1_1Book.html) for more details.   
For listing of Histogram types see [MarlinBookConfig.h](/doxygen/MarlinBookConfig_8h_source.html)   


```cpp
// dim… dimenision of data points(1, 2, and 3 are supported)
// w_t… type of weight, F… float, D… double, I… interger
Hist/*dim*//*w_t*/ hist = book/*dim*//*w_t*/(
	Processor* proc, 
	const std::filesystem::path& path,
	const std::string& name,
	const std::string& title,
	const Hist/*dim*//*w_t*/::AxisConfig_t& axis1,
	[const Hist/*dim*//*w_t*/::AxisConfig_t& axis2,]
	[const Hist/*dim*//*w_t*/::AxisConfig_t& axis3,]
	[BookFlag_t = BookFlags::Default]
);
```
search if histogram already exist, if not create new histogram, then return Entry to histogram.

## fill functions
```cpp
// add single point
handle.fill(Point_t, Weight_t)   

// fill multiple entries at one with fillN, use for this an array or vector data structure    
handle.fillN(std::array<Point_t>, std::array<Weight_t>)   
handle.fillN(std::vector<Point_t>, std::vector<Weight_t>)   
```

## handle function
ONLY use in processEvent, and only as locale variable!!

