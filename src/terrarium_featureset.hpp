#pragma once

#include "terrarium_datasource.hpp"
#include <mapnik/feature.hpp>
#include <mapnik/debug.hpp>
#include <vector>
#include <boost/utility.hpp>

namespace mapnik {

class terrarium_featureset : public mapnik::Featureset
{

public:
    terrarium_featureset(box2d<double> const& extent,
                      mapnik::query const& q,
                      std::shared_ptr<mapnik::image_reader> image_reader);
    virtual ~terrarium_featureset();
    mapnik::feature_ptr next();

private:
    mapnik::value_integer feature_id_;
    mapnik::context_ptr ctx_;
    mapnik::box2d<double> extent_;
    mapnik::box2d<double> bbox_;
    double filter_factor_;
    bool done = false;
    std::shared_ptr<mapnik::image_reader> image_reader_;
};

}

#if !defined(MAPNIK_TERRARIUM_LIBRARY)
#include "terrarium_featureset.ipp"
#endif