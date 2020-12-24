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
        int image_width = 516;
        int image_height = 516;

        if (image_width > 0 && image_height > 0)
        {
            mapnik::view_transform t(image_width, image_height, extent_, 0, 0);
            //box2d<double> intersect = bbox_.intersect(curIter_->envelope());
            box2d<double> intersect = extent_;
            box2d<double> ext = t.forward(intersect);
            box2d<double> rem = box2d<double>(0, 0, 0, 0);
            // select minimum raster containing whole ext
            int x_off = static_cast<int>(std::floor(ext.minx()));
            int y_off = static_cast<int>(std::floor(ext.miny()));
            int end_x = static_cast<int>(std::ceil(ext.maxx()));
            int end_y = static_cast<int>(std::ceil(ext.maxy()));

            // clip to available data
            if (x_off >= image_width) x_off = image_width - 1;
            if (y_off >= image_height) y_off = image_height - 1;
            if (x_off < 0) x_off = 0;
            if (y_off < 0) y_off = 0;
            if (end_x > image_width)  end_x = image_width;
            if (end_y > image_height) end_y = image_height;

            int width = end_x - x_off;
            int height = end_y - y_off;
            if (width < 1) width = 1;
            if (height < 1) height = 1;

            // calculate actual box2d of returned raster
            box2d<double> feature_raster_extent(rem.minx() + x_off,
                                                rem.miny() + y_off,
                                                rem.maxx() + x_off + width,
                                                rem.maxy() + y_off + height);
            feature_raster_extent = t.backward(feature_raster_extent);
            mapnik::image_any data = image_reader_->read(x_off, y_off, width, height);
            mapnik::raster_ptr raster = std::make_shared<mapnik::raster>(feature_raster_extent, intersect, std::move(data), filter_factor_);
            feature->set_raster(raster);
        }
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