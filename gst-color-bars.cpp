#include <gst/gst.h>
#include <string>
#include <iostream>
#include <format>

#include "gst-color-bars.h"

namespace {

    void log(std::string_view const& msg) {

        std::cout << std::format("{}", msg) << std::endl;
    }

    using namespace gst;

    class GstColorBarsImpl : public GstColorBars
    {
    public:
        GstColorBarsImpl(){
            log("GstColorBarsImpl::GstColorBarsImpl()");
        }

        ~GstColorBarsImpl() override {
            log("GstColorBarsImpl::~GstColorBarsImpl()");
        }

    };
}

namespace gst {

    GstColorBars *make_GstColorBars()
    {
        return new GstColorBarsImpl();
    }
}