//
// Created by yile0 on 2023/12/23.
//

#ifndef RESTREAMER_CREATETASKREQ_H
#define RESTREAMER_CREATETASKREQ_H

#include <optional>

#include "nlohmann/json.hpp"


#ifndef NLOHMANN_OPT_HELPER
#define NLOHMANN_OPT_HELPER
namespace nlohmann {
    template <typename T>
    struct adl_serializer<std::shared_ptr<T>> {
        static void to_json(json & j, std::shared_ptr<T> const & opt) {
            if (!opt) j = nullptr; else j = *opt;
        }

        static std::shared_ptr<T> from_json(json const & j) {
            if (j.is_null()) return std::make_shared<T>(); else return std::make_shared<T>(j.get<T>());
        }
    };
    template <typename T>
    struct adl_serializer<std::optional<T>> {
        static void to_json(json & j, std::optional<T> const & opt) {
            if (!opt) j = nullptr; else j = *opt;
        }

        static std::optional<T> from_json(json const & j) {
            if (j.is_null()) return std::make_optional<T>(); else return std::make_optional<T>(j.get<T>());
        }
    };
}
#endif

namespace CreateTaskReq {
    using nlohmann::json;

#ifndef NLOHMANN_UNTYPED_CreateTaskReq_HELPER
#define NLOHMANN_UNTYPED_CreateTaskReq_HELPER
    inline json get_untyped(json const & j, char const * property) {
        if (j.find(property) != j.end()) {
            return j.at(property).get<json>();
        }
        return json();
    }

    inline json get_untyped(json const & j, std::string property) {
        return get_untyped(j, property.data());
    }
#endif

#ifndef NLOHMANN_OPTIONAL_CreateTaskReq_HELPER
#define NLOHMANN_OPTIONAL_CreateTaskReq_HELPER
    template <typename T>
    inline std::shared_ptr<T> get_heap_optional(json const & j, char const * property) {
        auto it = j.find(property);
        if (it != j.end() && !it->is_null()) {
            return j.at(property).get<std::shared_ptr<T>>();
        }
        return std::shared_ptr<T>();
    }

    template <typename T>
    inline std::shared_ptr<T> get_heap_optional(json const & j, std::string property) {
        return get_heap_optional<T>(j, property.data());
    }
    template <typename T>
    inline std::optional<T> get_stack_optional(json const & j, char const * property) {
        auto it = j.find(property);
        if (it != j.end() && !it->is_null()) {
            return j.at(property).get<std::optional<T>>();
        }
        return std::optional<T>();
    }

    template <typename T>
    inline std::optional<T> get_stack_optional(json const & j, std::string property) {
        return get_stack_optional<T>(j, property.data());
    }
#endif

    struct Ext {
    };

    struct BgAudio {
        std::optional<std::string> url;
        std::optional<double> volume;
        std::optional<double> duration;
    };

    struct BgImage {
        std::optional<int64_t> display;
        std::optional<std::string> url;
        std::optional<int64_t> left;
        std::optional<int64_t> right;
        std::optional<int64_t> top;
        std::optional<int64_t> bottom;
        std::optional<int64_t> level;
        std::optional<int64_t> is_loop;
        std::optional<float> offset;
        std::optional<int64_t> is_gif;
    };

    struct Videos {
        std::optional<int64_t> video_type;
        std::optional<std::string> main_video_url;
        std::optional<std::string> mask_video_url;
        std::optional<int64_t> enable_coord;
        std::optional<int64_t> left;
        std::optional<int64_t> right;
        std::optional<int64_t> top;
        std::optional<int64_t> bottom;
        std::optional<int64_t> level;
    };

    struct Source {
        std::optional<int64_t> index;
        std::optional<Videos> videos;
        std::optional<BgImage> bg_image;
        std::optional<std::vector<BgImage>> patches;
        std::optional<BgImage> close_shot;
        std::optional<BgAudio> bg_audio;
    };

    struct Data {
        std::optional<std::string> task_name;
        std::optional<std::string> start_time;
        std::optional<std::string> end_time;
        std::optional<std::string> width;
        std::optional<std::string> height;
        std::optional<std::vector<Source>> sources;
        std::optional<std::string> callback_url;
        std::optional<std::string> rtmp;
        std::optional<Ext> ext;
    };

    struct CreateTaskReq {
        std::optional<std::string> appid;
        std::optional<int64_t> timestamp;
        std::optional<std::string> version;
        std::optional<std::string> sign;
        std::optional<Data> data;
    };
}

namespace CreateTaskReq {
    void from_json(json const & j, Ext & x);
    void to_json(json & j, Ext const & x);

    void from_json(json const & j, BgAudio & x);
    void to_json(json & j, BgAudio const & x);

    void from_json(json const & j, BgImage & x);
    void to_json(json & j, BgImage const & x);

    void from_json(json const & j, Videos & x);
    void to_json(json & j, Videos const & x);

    void from_json(json const & j, Source & x);
    void to_json(json & j, Source const & x);

    void from_json(json const & j, Data & x);
    void to_json(json & j, Data const & x);

    void from_json(json const & j, CreateTaskReq & x);
    void to_json(json & j, CreateTaskReq const & x);

    inline void from_json(json const & j, Ext& x) {
    }

    inline void to_json(json & j, Ext const & x) {
        j = json::object();
    }

    inline void from_json(json const & j, BgAudio& x) {
        x.url = get_stack_optional<std::string>(j, "url");
        x.volume = get_stack_optional<double>(j, "volume");
        x.duration = get_stack_optional<double>(j, "duration");
    }

    inline void to_json(json & j, BgAudio const & x) {
        j = json::object();
        if (x.url) {
            j["url"] = x.url;
        }
        if (x.volume) {
            j["volume"] = x.volume;
        }
        if (x.duration) {
            j["duration"] = x.duration;
        }
    }

    inline void from_json(json const & j, BgImage& x) {
        x.display = get_stack_optional<int64_t>(j, "display");
        x.url = get_stack_optional<std::string>(j, "url");
        x.left = get_stack_optional<int64_t>(j, "left");
        x.right = get_stack_optional<int64_t>(j, "right");
        x.top = get_stack_optional<int64_t>(j, "top");
        x.bottom = get_stack_optional<int64_t>(j, "bottom");
        x.level = get_stack_optional<int64_t>(j, "level");
        x.is_loop = get_stack_optional<int64_t>(j, "is_loop");
        x.offset = get_stack_optional<float>(j, "offset");
        x.is_gif = get_stack_optional<int64_t>(j, "is_gif");
    }

    inline void to_json(json & j, BgImage const & x) {
        j = json::object();
        if (x.display) {
            j["display"] = x.display;
        }
        if (x.url) {
            j["url"] = x.url;
        }
        if (x.left) {
            j["left"] = x.left;
        }
        if (x.right) {
            j["right"] = x.right;
        }
        if (x.top) {
            j["top"] = x.top;
        }
        if (x.bottom) {
            j["bottom"] = x.bottom;
        }
        if (x.level) {
            j["level"] = x.level;
        }
        if (x.is_loop) {
            j["is_loop"] = x.is_loop;
        }
        if (x.offset) {
            j["offset"] = x.offset;
        }
        if (x.is_gif) {
            j["is_gif"] = x.is_gif;
        }
    }

    inline void from_json(json const & j, Videos& x) {
        x.video_type = get_stack_optional<int64_t>(j, "video_type");
        x.main_video_url = get_stack_optional<std::string>(j, "main_video_url");
        x.mask_video_url = get_stack_optional<std::string>(j, "mask_video_url");
        x.enable_coord = get_stack_optional<int64_t>(j, "enable_coord");
        x.left = get_stack_optional<int64_t>(j, "left");
        x.right = get_stack_optional<int64_t>(j, "right");
        x.top = get_stack_optional<int64_t>(j, "top");
        x.bottom = get_stack_optional<int64_t>(j, "bottom");
        x.level = get_stack_optional<int64_t>(j, "level");
    }

    inline void to_json(json & j, Videos const & x) {
        j = json::object();
        if (x.video_type) {
            j["video_type"] = x.video_type;
        }
        if (x.main_video_url) {
            j["main_video_url"] = x.main_video_url;
        }
        if (x.mask_video_url) {
            j["mask_video_url"] = x.mask_video_url;
        }
        if (x.enable_coord) {
            j["enable_coord"] = x.enable_coord;
        }
        if (x.left) {
            j["left"] = x.left;
        }
        if (x.right) {
            j["right"] = x.right;
        }
        if (x.top) {
            j["top"] = x.top;
        }
        if (x.bottom) {
            j["bottom"] = x.bottom;
        }
        if (x.level) {
            j["level"] = x.level;
        }
    }

    inline void from_json(json const & j, Source& x) {
        x.index = get_stack_optional<int64_t>(j, "index");
        x.videos = get_stack_optional<Videos>(j, "videos");
        x.bg_image = get_stack_optional<BgImage>(j, "bg_image");
        x.patches = get_stack_optional<std::vector<BgImage>>(j, "patches");
        x.close_shot = get_stack_optional<BgImage>(j, "close_shot");
        x.bg_audio = get_stack_optional<BgAudio>(j, "bg_audio");
    }

    inline void to_json(json & j, Source const & x) {
        j = json::object();
        if (x.index) {
            j["index"] = x.index;
        }
        if (x.videos) {
            j["videos"] = x.videos;
        }
        if (x.bg_image) {
            j["bg_image"] = x.bg_image;
        }
        if (x.patches) {
            j["patches"] = x.patches;
        }
        if (x.close_shot) {
            j["close_shot"] = x.close_shot;
        }
        if (x.bg_audio) {
            j["bg_audio"] = x.bg_audio;
        }
    }

    inline void from_json(json const & j, Data& x) {
        x.task_name = get_stack_optional<std::string>(j, "task_name");
        x.start_time = get_stack_optional<std::string>(j, "start_time");
        x.end_time = get_stack_optional<std::string>(j, "end_time");
        x.width = get_stack_optional<std::string>(j, "width");
        x.height = get_stack_optional<std::string>(j, "height");
        x.sources = get_stack_optional<std::vector<Source>>(j, "sources");
        x.callback_url = get_stack_optional<std::string>(j, "callback_url");
        x.rtmp = get_stack_optional<std::string>(j, "rtmp");
        x.ext = get_stack_optional<Ext>(j, "ext");
    }

    inline void to_json(json & j, Data const & x) {
        j = json::object();
        if (x.task_name) {
            j["task_name"] = x.task_name;
        }
        if (x.start_time) {
            j["start_time"] = x.start_time;
        }
        if (x.end_time) {
            j["end_time"] = x.end_time;
        }
        if (x.width) {
            j["width"] = x.width;
        }
        if (x.height) {
            j["height"] = x.height;
        }
        if (x.sources) {
            j["sources"] = x.sources;
        }
        if (x.callback_url) {
            j["callback_url"] = x.callback_url;
        }
        if (x.rtmp) {
            j["rtmp"] = x.rtmp;
        }
        if (x.ext) {
            j["ext"] = x.ext;
        }
    }

    inline void from_json(json const & j, CreateTaskReq& x) {
        x.appid = get_stack_optional<std::string>(j, "appid");
        x.timestamp = get_stack_optional<int64_t>(j, "timestamp");
        x.version = get_stack_optional<std::string>(j, "version");
        x.sign = get_stack_optional<std::string>(j, "sign");
        x.data = get_stack_optional<Data>(j, "data");
    }

    inline void to_json(json & j, CreateTaskReq const & x) {
        j = json::object();
        if (x.appid) {
            j["appid"] = x.appid;
        }
        if (x.timestamp) {
            j["timestamp"] = x.timestamp;
        }
        if (x.version) {
            j["version"] = x.version;
        }
        if (x.sign) {
            j["sign"] = x.sign;
        }
        if (x.data) {
            j["data"] = x.data;
        }
    }
}



#endif  // RESTREAMER_CREATETASKREQ_H
