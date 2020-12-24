#pragma once

#include <mapnik/datasource.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry/box2d.hpp>

#include <boost/optional.hpp>
#include <memory>

#include <vector>
#include <string>

namespace mapnik {

class terrarium_datasource : public mapnik::datasource
{
public:
    terrarium_datasource(std::shared_ptr<mapnik::image_reader> image_reader, std::uint64_t x,std::uint64_t y,std::uint64_t z);
    virtual ~terrarium_datasource();
    datasource::datasource_t type() const;
    static const char * name();
    mapnik::featureset_ptr features(const mapnik::query& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    box2d<double> get_tile_extent() const;
    boost::optional<mapnik::datasource_geometry_t> get_geometry_type() const;
    mapnik::layer_descriptor get_descriptor() const;

private:
    mapnik::layer_descriptor desc_;
    mutable mapnik::box2d<double> extent_;
    mutable bool extent_initialized_;
    unsigned width_;
    unsigned height_;
    std::uint64_t x_;
    std::uint64_t y_;
    std::uint64_t z_;
    std::shared_ptr<mapnik::image_reader> image_reader_;
};

}

#if !defined(MAPNIK_TERRARIUM_LIBRARY)
#include "terrarium_datasource.ipp"
#endif
