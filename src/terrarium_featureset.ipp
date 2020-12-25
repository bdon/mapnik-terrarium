#include <mapnik/debug.hpp>
#include <mapnik/image.hpp>
#include <mapnik/raster.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/util/variant.hpp>

#include "terrarium_featureset.hpp"

using mapnik::query;
using mapnik::image_reader;
using mapnik::feature_ptr;
using mapnik::image_rgba8;
using mapnik::raster;
using mapnik::feature_factory;

namespace mapnik {

terrarium_featureset::terrarium_featureset(box2d<double> const& extent,
                                           query const& q,
                                           std::shared_ptr<mapnik::image_reader> image_reader)
    : feature_id_(1),
      ctx_(std::make_shared<mapnik::context_type>()),
      extent_(extent),
      bbox_(q.get_bbox()),
      filter_factor_(q.get_filter_factor()),
      image_reader_(image_reader)
{
}

terrarium_featureset::~terrarium_featureset()
{
}

feature_ptr terrarium_featureset::next()
{
    if (done) return feature_ptr();
    feature_ptr feature(feature_factory::create(ctx_,feature_id_++));
    try
    {
        // TODO for zooms > 16 need to reintroduce overzooming
        mapnik::image_any data = image_reader_->read(2, 2, 512, 512);
        mapnik::raster_ptr raster = std::make_shared<mapnik::raster>(extent_, extent_, std::move(data), filter_factor_);
        feature->set_raster(raster);
    }
    catch (mapnik::image_reader_exception const& ex)
    {
        MAPNIK_LOG_ERROR(raster) << "Terrarium: image reader exception caught: " << ex.what();
    }
    catch (std::exception const& ex)
    {
        MAPNIK_LOG_ERROR(raster) << "Terrarium: " << ex.what();
    }
    catch (...)
    {
        MAPNIK_LOG_ERROR(raster) << "Terrarium: exception caught";
    }

    done = true;
    return feature;
}

}