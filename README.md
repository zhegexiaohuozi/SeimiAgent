# SeimiAgent #
A headless,standalone webkit server which make grabing dynamic web page easier.

# DownLoad #
[Centos6 x64](http://seimidl.wanghaomiao.cn/seimiagent_for_centos6_x64_v1.0.tar.gz)
[ubuntu x64](http://seimidl.wanghaomiao.cn/seimiagent_for_ubuntu_x64_v1.0.tar.gz)

# Quick Start #
```
cd /dir/of/seimiAgent
./SeimiAgent -p 8000
```
SeimiAgent will startup and listen on the port that you set.Than you can use any http client tools post a load reqest to SeimiAgent and get back the content which just like chrome do.Http client tools you can use:
apache `httpclient` of java,`curl` of cmd,`httplib2` of python including, but not limited to.

## Http parameters that seimiAgent support ##
- `url`
your target url

- `renderTime`
How long time you hope to give seimiAgent to process javascript action and document after load finashed.

- `proxy`
Tell SeimiAgent to use proxy.Pattern:`http|https|socket://user:passwd@host:port`

- `postParam`
Json string only,tell seimiAgent you want to use http post method and pass the parameters in `postParam`.

- `useCookie`
If `useCookie`==1,seimiAgent deem you want to use cookie.Default 0.

# More #
More Doc is on his way...
