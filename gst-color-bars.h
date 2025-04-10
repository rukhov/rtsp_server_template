#pragma once

namespace gst {

    class GstColorBars {
        public:
        virtual ~GstColorBars() = default;

        static void register_element();
    };

    GstColorBars *make_GstColorBars();
}