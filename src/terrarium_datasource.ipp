#include <mapnik/debug.hpp>
#include <mapnik/view_transform.hpp>
#include "terrarium_featureset.hpp"
#include "terrarium_datasource.hpp"

using mapnik::layer_descriptor;
using mapnik::featureset_ptr;
using mapnik::query;
using mapnik::coord2d;
using mapnik::datasource;
using mapnik::parameters;

namespace mapnik {

inline mapnik::box2d<double> tile_mercator_bbox(std::uint64_t x,
                                                std::uint64_t y,
                                                std::uint64_t z)
{
    const double half_of_equator = M_PI * EARTH_RADIUS;
    const double tile_size = 2.0 * half_of_equator / (1ull << z);
    double minx = -half_of_equator + x * tile_size;
    double miny = half_of_equator - (y + 1.0) * tile_size;
    double maxx = -half_of_equator + (x + 1.0) * tile_size;
    double maxy = half_of_equator - y * tile_size;
    return mapnik::box2d<double>(minx,miny,maxx,maxy);
}


terrarium_datasource::terrarium_datasource(
    std::shared_ptr<mapnik::image_reader> image_reader,
    std::uint64_t x,
    std::uint64_t y,
    std::uint64_t z
  ) : datasource(parameters()),
    desc_("in-memory RGB encoded datasource", "utf-8"),
    extent_initialized_(false),
    image_reader_(image_reader),
    x_(x),
    y_(y),
    z_(z)
{
    MAPNIK_LOG_DEBUG(raster) << "terrarium_datasource: Initializing...";
}

terrarium_datasource::~terrarium_datasource()
{
}

mapnik::datasource::datasource_t terrarium_datasource::type() const
{
    return datasource::Raster;
}

const char * terrarium_datasource::name()
{
    return "terrarium";
}

box2d<double> terrarium_datasource::get_tile_extent() const
{
    return tile_mercator_bbox(x_, y_, z_);
}

mapnik::box2d<double> terrarium_datasource::envelope() const
{
    if (!extent_initialized_)
    {
        extent_ = get_tile_extent();
        extent_initialized_ = true;
    }
    return extent_;
}

boost::optional<mapnik::datasource_geometry_t> terrarium_datasource::get_geometry_type() const
{
    return boost::optional<mapnik::datasource_geometry_t>();
}

layer_descriptor terrarium_datasource::get_descriptor() const
{
    return desc_;
}

featureset_ptr terrarium_datasource::features(query const& q) const
{
    return std::make_shared<terrarium_featureset>(extent_, q, image_reader_);
}

featureset_ptr terrarium_datasource::features_at_point(coord2d const&, double tol) const
{
    MAPNIK_LOG_WARN(raster) << "terrarium_datasource: feature_at_point not supported";

    return mapnik::make_invalid_featureset();
}

}