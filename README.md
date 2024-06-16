# reStreamer

## 概述
这款多媒体融合流媒体服务器提供了一站式的媒体处理和推流解决方案,能够将音频、视频和图片等多种媒体资源进行融合并实时推送至流媒体服务器,非常适合内容创作者和媒体运营商使用。
### 主要功能
- 媒体文件导入:支持主流的音频、视频和图片格式,用户可以方便地上传素材文件。
- 媒体融合处理:采用先进的流媒体处理技术,将不同类型的媒体文件进行时间轴对齐和场景融合,保证音视频同步,图片素材能够适时插入。
- 实时推流:将处理后的媒体流通过RTMP、HLS、DASH等主流流媒体协议实时推送至流媒体服务器。
- 控制台API:提供Web API,用户可以上传素材、调整推流参数、监控推流状态等。

应用场景
适用于: 
- 视频直播
- 在线课堂
- 活动直播
- 广告营销等场景

能够帮助用户将各种类型的媒体资源融合为丰富多彩的直播和点播内容。

## 编译C++部分

### 安装C++工具链

```sh
sudo apt update
sudo apt install build-essential
sudo apt install cmake
```

### 安装vcpkg

#### 克隆 vcpkg 存储库

```sh
git clone https://github.com/Microsoft/vcpkg.git
```

#### 运行引导脚本以生成 vcpkg

```sh
 .\vcpkg\bootstrap-vcpkg.sh
```

安装成功

```sh
Downloading vcpkg-glibc...
vcpkg package management program version 2023-12-12-1c9ec1978a6b0c2b39c9e9554a96e3e275f7556e

See LICENSE.txt for license information.
Telemetry
---------
vcpkg collects usage data in order to help us improve your experience.
The data collected by Microsoft is anonymous.
You can opt-out of telemetry by re-_running the bootstrap-vcpkg script with -disableMetrics,
passing --disable-metrics to vcpkg on the command line,
or by setting the VCPKG_DISABLE_METRICS environment variable.

Read more about vcpkg telemetry at docs/about/privacy.md
```

### 安装依赖

```sh
 ./vcpkg install spdlog
 ./vcpkg install ffmpeg[ffmpeg,avcodec,avdevice,avfilter,avformat,avresample,swresample,swscale,,nvcodec,x264,x265,vpx,webp,vorbis,opus,zlib]
 ./vcpkg install nlohmann-json
 ./vcpkg install openssl
```

### 编译代码

进入代码目录，创建编译目录:

```sh
mkdir build
```

生产Makefile

```sh
cd build
cmake .. -DVCPKG_ROOT=/home/ubuntu/vcpkg
```

编译

```sh
make
```

### 运行

#### 导出动态库

```sh
 ./vcpkg export --raw spdlog ffmpeg nlohmann-json openssl
```

上面步骤会将依赖的库导出到当前目录，进入可执行库目录，将动态库加入到环境变量。

#### Run

```sh
./reStreamer
```
