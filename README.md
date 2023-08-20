# TinyHttpServer
tiny c++ http server implementation with asio

# Dependencies

- asio (without boost)
- spdlog
- json

# TODO

- [x] Add HTTP/1.0 (just GET request) Demo from boost::asio example
- [ ] Add HTTP/1.1 implementation (GET + POST)
- [ ] Add HTTPS implementation with asio::ssl
- [ ] Add mysql connection (c++ connector) like TinyWebServer
- [ ] Add pressure test compared with [TinyWebServer](https://github.com/qinguoyi/TinyWebServer)