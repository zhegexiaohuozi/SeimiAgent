# SeimiAgent #
SeimiAgent是基于QtWebkit开发的可在服务器端后台运行的一个webkit服务，可以通过SeimiAgent提供的http接口向SeimiAgent发送一个load请求（需求加载的URL以及对这个页面接受的渲染时间或是使用什么代理等参数），通过SeimiAgent去加载并渲染想要处理的动态页面，然后将渲染好的页面直接返给调用方进行后续处理，所以运行的SeimiAgent服务是与语言无关的，任何一种语言或框架都可以通过SeimiAgent提供的标准http接口来获取服务。SeimiAgent的加载渲染环境都是通用浏览器级的，所以不用担心他对动态页面的处理能力。同时支持渲染生成页面快照（png）和PDF，亦支持自定义js脚本处理基本渲染后的页面，具体请参见下方使用说明。

# 快速开始 #
```
cd /dir/of/seimiAgent
./seimiagent -p 8000
```
执行命令后，SeimiAgent会起一个http服务并监听你所指定的端口，如例子中的8000端口，然后你就可以通过任何一种你熟悉的语言像SeimiAgent发送一个页面的加载渲染请求，并得到SeimiAgent渲染好的HTML文档进行后续处理。

## 示例 ##
![demo](http://img.wanghaomiao.cn/seimiagent/demo.gif)

## 支持的http参数 ##
仅支持post请求,请求地址`/doload`
- `url`
目标请求地址，必填项

- `renderTime`
在所有资源都加载好了以后留给SeimiAgent去渲染处理的时间，如果是很复杂的动态页面这个时间可能就需要长一些，具体根据使用情况进行调整。非必填,单位为毫秒

- `proxy`
告诉SeimiAgent使用什么代理，非必填，格式:`http|https|socket://user:passwd@host:port`

- `postParam`
这个参数只接受Json格式的值，值的形式为key-value对，告诉SeimiAgent此次请求为post并使用你给定的参数。

- `useCookie`
是否使用cookie，如果设置为1则为使用cookie

- `contentType`
定义渲染结果的生成格式，可以选择的值有`img`或是`pdf`，默认值为`html`。


- `script`
可以传一段js脚本并在渲染好页面后执行，就像是在chrome的控制台中执行的一样。

- `ua`
自定义一个UserAgent，如果你需要

- `resourceTimeout`
设置资源拉取的超时时间，如js等资源。默认20s。

# 如何构建 #
这个过程会花费很长时间如果你觉着很有必要的话，一般情况下更推荐使用发布好的二进制可执行文件

## 依赖 ##
- ubuntu上
```
sudo apt-get install build-essential g++ flex bison gperf ruby perl libsqlite3-dev libfontconfig1-dev libicu-dev libfreetype6 libssl-dev libpng-dev libjpeg-dev python libx11-dev libxext-dev
```

- centos上
```
yum -y install gcc gcc-c++ make flex bison gperf ruby openssl-devel freetype-devel fontconfig-devel libicu-devel sqlite-devel libpng-devel libjpeg-devel
```
## 执行 ##
```
python build.py
```
接下来就等吧，国内网络不好可能还要重来（因为需要先从github上下载qtbase和qtwebkit这两个依赖，后续如果有时间会把qtbase和qtwebkit拷到国内仓库一份），4核I5大概半个小时以上，单核云主机一般2个小时左右，16核以上服务器编译一般在十分钟以内

# More #
更多文档还在准备中，感谢大家支持Seimi家族([SeimiCrawler](https://github.com/zhegexiaohuozi/SeimiCrawler),[SeimiAgent](https://github.com/zhegexiaohuozi/SeimiAgent))

# Change Log #

## v1.2 ##

- 支持执行自定义的js脚本

- 默认发行包去掉对`libwebp`的依赖，使之适用平台范围更广

- 优化图片以及PDF的渲染

## v1.1 ##
- 支持通过`contentType`参数来指定生成的结果的文件格式，新增图片和PDF两种格式的支持。可以通过`img`或是`pdf`这两个值来指定。

- 添加Linux 64位平台通用包
