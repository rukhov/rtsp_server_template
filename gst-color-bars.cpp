#include <gst/gst.h>

#include "gst-color-bars.h"

namespace {

    using namespace gst;

    class GstColorBarsImpl : public GstColorBars
    {
    public:
        GstColorBarsImpl(){

        }

        ~GstColorBarsImpl() override {

        }

    };
}

namespace gst {

    GstColorBars *make_GstColorBars()
    {
        return new GstColorBarsImpl();
    }
}