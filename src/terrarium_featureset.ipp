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

double height_val(uint32_t pixel) {
    uint8_t red = pixel & 0xff;
    uint8_t green = (pixel >> 8) & 0xff;
    uint8_t blue = (pixel >> 16) & 0xff;
    // https://github.com/tilezen/joerd/blob/master/docs/formats.md
    return (red * 256 + green + blue / 256) - 32768;
}

uint32_t pxl_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint32_t val = (a << 24);
    val = val | (b << 16);
    val = val | (g << 8);
    val = val | r;
    return val;
}

void process(mapnik::image_rgba8 const& input, mapnik::image_rgba8 &output) {
    for (int row_idx = 0; row_idx < 512; row_idx++) {
        auto row = input.get_row(row_idx,2);
        uint32_t *buf = new uint32_t[512];
        for (int col = 0; col < 512; col++) {
            double hgt = height_val(row[col+2]);
            double frac = hgt / 1000; // arbitrary number
            if (frac < 0) frac = 0.0;
            if (frac > 1) frac = 1.0;
            frac = frac * 255;
            uint8_t v = frac;
            buf[col] = pxl_from_rgba(0,0,255,v);
        }
        output.set_row(row_idx, buf, 512);
    }
}

feature_ptr terrarium_featureset::next()
{
    if (done) return feature_ptr();
    feature_ptr feature(feature_factory::create(ctx_,feature_id_++));
    try
    {
        // TODO for zooms > 16 need to reintroduce overzooming
        mapnik::image_any input = image_reader_->read(0, 0, 516, 516);
        mapnik::image_rgba8 output(512,512);
        auto const &input_img = input.get<mapnik::image_rgba8>();
        process(input_img,output);
        mapnik::raster_ptr raster = std::make_shared<mapnik::raster>(extent_, extent_, std::move(output), filter_factor_);
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